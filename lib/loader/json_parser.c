#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <json/json.h>
#include "json_parser.h"
#include "json_device.h"

#define DYNAMIC_JSON_DEBUG

/* qemu is compiled by c90 */
#ifdef json_object_object_foreach
#undef json_object_object_foreach
#define json_object_object_foreach(obj, key, val) \
	char *key; struct json_object *val; struct lh_entry *entry; \
	for(entry = json_object_get_object(obj)->head; ({ if(entry) { key = (char*)entry->k; val = (struct json_object*)entry->v; } ; entry; }); entry = entry->next )
#endif

#ifndef __G_LIB_H__
#include <stdlib.h>
#include <string.h>
#define g_renew(type, var, len) \
	realloc((void*)(var), (len) * sizeof(type))
#define g_new(type, len) \
	malloc((len) * sizeof(type))
#define g_free(var) \
	free(var)
#define g_strdup(str) \
	strdup(str)
#endif

static int calculate_exp(const char *str, int *r,
		int start, int end,
		const char *index_ident, int index)
{
	int i, j;
	int n = 0;
	int sum = 0;
	int status = 1; /* SUB op is 0, ADD op is 1 */

	if(index_ident == NULL) {
		fprintf(stderr, "Maybe something wrong have happend.\n");
		return -1;
	}
	if(index_ident[0] == '\0') {
		fprintf(stderr, "index setting wrong.\n");
		return -1;
	}

	for(i = start; i < end; i++) {
		switch(str[i]) {
			case '$':
			{
				for(j = 0; index_ident[j]; j++) {
					if(str[i + 1 + j] - index_ident[j]) {
						break;
					}
				}
				if(index_ident[j] == '\0') {
					sum += index;
					i += strlen(index_ident);
					break;
				}else{
					fprintf(stderr, "have a error identify \"%s\"\n", str);
					return -1;
				}
			}
			case '0' ... '9':
				n *= 10;
				n += str[i] - '0';
				break;
			case '+':
				status = 1;
				break;
			case '-':
				status = 0;
				break;
			case ' ':
				if(status)
					sum += n;
				else
					sum -= n;
				n = 0;
				break;
			default:
				fprintf(stderr, "undefined character \"%c\"\n", str[i]);
				return -1;
		}
	}
	if(i >= end) {
		if(status)
			sum += n;
		else
			sum -= n;
	}
	*r = sum;
	return 0;
}

static char* deal_str(const char *str, const char *index_ident, int index)
{
	int i, j, t, len = 0;
	int exp_start = 0, exp_end = 0;
	bool exp_mode = false;
	char *buf = NULL;
	char numstr[32];

	/* deal special charactor and copy them to buf */
	/* # $i + 1 # is a expression */
	for(i = 0; str[i] != '\0'; i++) {
		switch(str[i]) {
			case '#':
			{
				if(exp_mode) {
					exp_end = i;
					int n;
					t = calculate_exp(str, &n,
							exp_start, exp_end,
							index_ident, index);
					if(t) {
						g_free(buf);
						return NULL;
					}
					/* integer translate to string and copy it to buf */
					sprintf(numstr, "%d", n);
					n = strlen(numstr);
					buf = g_renew(char, buf, len + n);
					for(j = 0; j < n; j++) {
						buf[len + j] = numstr[j];
					}
					len += n;
					exp_mode = false;
					break;
				}else{
					if(str[i + 1] != '#') {
						/* "#" mean special mode start */
						exp_start = i + 1;
						exp_mode = true;
						break;
					}else{
						/* "##" mean "#" */
						i++;
					}
				}
			}
			default:
			{
				if(!exp_mode) {
					buf = g_renew(char, buf, len + 1);
					buf[len] = str[i];
					len++;
				}
			}
		}
	}

	buf = g_renew(char, buf, len + 1);
	buf[len] = '\0';
	return buf;
}

static char* file_get_string(const char *path)
{
	int fd, ret, len;
	char *buf;

	fd = open(path, O_RDONLY);
	if(fd < 0) {
		perror("open conf file");
		return NULL;
	}

	len = lseek(fd, 0, SEEK_END);
	buf = (char*) g_new(char, len);

	lseek(fd, 0, SEEK_SET);
	ret = read(fd, buf, len - 1);
	if(ret < 0) {
		perror("read conf file");
		return NULL;
	}

	buf[len - 1] = '\0';
	return buf;
}

static int parse_irq(struct devargs *dev, struct json_object *interrupt,
		const char *index_ident, int index)
{
	struct irq_intf *irqlist = dev->irqlist;
	int i, j;
	int irq_num = dev->irq_num;
	char *tmp_str;

	json_object_object_foreach(interrupt, irq_dev, irq) {
		int len = json_object_array_length(irq);
		/* maybe have many irq num in a dev */
		for(i = 0; i < len; i++) {
			struct json_object *num = json_object_array_get_idx(irq, i);
			if(!num) {
				fprintf(stderr, "Get %s interrupt %s, %d interface "
						"failed from conf\n", dev->name, irq_dev, irq_num);
				//json_object_put(irq);
				goto err_happend;
			}

			irqlist = g_renew(struct irq_intf, irqlist, irq_num + 1);

			/* setting irq dev */
			tmp_str = deal_str(irq_dev, index_ident, index);
			if(!tmp_str) {
				fprintf(stderr, "%s interrupt %s, %d interface "
						"get a error dev name from conf\n", dev->name,
						irq_dev, irq_num);
				goto err_happend;
			}
			irqlist[irq_num].dev = tmp_str;

			/* setting irq num */
			json_type type = json_object_get_type(num);
			if(type == json_type_string) {
				tmp_str = deal_str(json_object_get_string(num), index_ident, index);
				if(!tmp_str) {
					fprintf(stderr, "%s interrupt %s, %d interface "
							"get a error dev irq num from conf\n", dev->name,
							irq_dev, irq_num);
					goto err_happend;
				}
				irqlist[irq_num].num = strtol(tmp_str, NULL, 10);
				g_free(tmp_str);
			}else if(type == json_type_int) {
				irqlist[irq_num].num = json_object_get_int(num);
			}else{
				fprintf(stderr, "Get %s interrupt %s, %d interface "
						"is a error type from conf\n", dev->name, irq_dev, irq_num);
				//json_object_put(irq);
				goto err_happend;
			}

			irq_num++;
			//json_object_put(num);
		}
		//json_object_put(irq);
	}
	dev->irq_num = irq_num;
	dev->irqlist = irqlist;
	return 0;
err_happend:
	for(j = 0; j < irq_num; j++) {
		g_free(irqlist[j].dev);
	}
	g_free(irqlist);
	return -1;
}

static int parse_con(struct devargs *dev, struct json_object *connect,
		const char *index_ident, int index)
{
	struct con_intf *conlist = dev->conlist;
	int i, j;
	int con_num = dev->con_num;
	char *tmp_str;

	json_object_object_foreach(connect, con_dev, con) {
		int len = json_object_array_length(con);
		/* maybe have many con in a dev */
		for(i = 0; i < len; i++) {
			struct json_object *con_con = json_object_array_get_idx(con, i);
			if(!con_con) {
				fprintf(stderr, "Get %s connect %s, %d interface "
						"failed from conf\n", dev->name, con_dev, con_num);
				//json_object_put(irq);
				goto err_happend;
			}

			conlist = g_renew(struct con_intf, conlist, con_num + 1);
			/* setting con dev */
			tmp_str = deal_str(con_dev, index_ident, index);
			if(!tmp_str) {
				fprintf(stderr, "%s connect %s, %d interface "
						"get a error dev name from conf\n", dev->name,
						con_dev, con_num);
				goto err_happend;
			}
			conlist[con_num].dev = tmp_str;

			/* setting con con */
			tmp_str = deal_str(json_object_get_string(con_con), index_ident, index);
			if(!tmp_str) {
				fprintf(stderr, "%s connect %s, %d interface "
						"get a error dev con con from conf\n", dev->name,
						con_dev, con_num);
				goto err_happend;
			}
			conlist[con_num].con = tmp_str;
			con_num++;
		}
		//json_object_put(con);
	}
	dev->con_num = con_num;
	dev->conlist = conlist;
	return 0;
err_happend:
	for(j = 0; j < con_num; j++) {
		g_free(conlist[j].dev);
		g_free(conlist[j].con);
	}
	g_free(conlist);
	return -1;
}

static int parse_dev(struct devargs **old_list, int n,
		struct json_object *json_dev, const char *name,
		const char *index_ident, int index)
{
#define PARSE_DEV_ERROR(step) \
	do { \
		err = -1; \
		goto err_##step; \
	}while(0)
#define PARSE_DEV_OBJ(step) \
	struct json_object *step; \
	step = json_object_object_get(json_dev, #step); \
	if(step)

	int i = 0;
	int ret, err = 0;
	struct devargs *dev, *list = *old_list;
	bool redefine = false;

	if(list && n > 0) {
		for(i = 0; i < n; i++) {
			if(strcmp(name, list[i].name) == 0) {
				redefine = true;
				dev = list + i;
				break;
			}
		}
	}
	if(!redefine) {
		list = g_renew(struct devargs, list, n + 1);
		list[n].name  = g_strdup(name);
		list[n].devclass = NULL;
		list[n].has_addr = false;
		list[n].irqlist = NULL;
		list[n].conlist = NULL;
		list[n].irq_num = list[n].con_num = 0;
		dev = list + n;
		err = 1;
	}

	PARSE_DEV_OBJ(class) {
		if(redefine) {
			if(dev->devclass) {
				fprintf(stderr, "%s class have defined on top.\n", dev->name);
				PARSE_DEV_ERROR(class);
			}
		}
		dev->devclass = g_strdup(json_object_get_string(class));
	}else{
		if(!redefine) {
			fprintf(stderr, "Get %s class failed from conf\n", dev->name);
			//PARSE_DEV_ERROR(class);
		}
	}

	PARSE_DEV_OBJ(address) {
		if(redefine) {
			if(dev->has_addr) {
				fprintf(stderr, "%s address have defined on top.\n", dev->name);
				PARSE_DEV_ERROR(addr);
			}
		}
		dev->has_addr = true;
		char *tmp_str = deal_str(json_object_get_string(address), index_ident, index);
		if(!tmp_str) {
			fprintf(stderr, "%s address is a error string.\n", dev->name);
			PARSE_DEV_ERROR(addr);
		}
		dev->address = strtoul(tmp_str, NULL, 0);
	}else{
		dev->has_addr = false;
	}

	PARSE_DEV_OBJ(length) {
		if(redefine) {
			if(dev->has_len) {
				fprintf(stderr, "%s length have defined on top.\n", dev->name);
				PARSE_DEV_ERROR(length);
			}
		}
		dev->has_len = true;
		char *tmp_str = deal_str(json_object_get_string(length), index_ident, index);
		if(!tmp_str) {
			fprintf(stderr, "%s length is a error string.\n", dev->name);
			PARSE_DEV_ERROR(length);
		}
		dev->length = strtoul(tmp_str, NULL, 0);
	}else{
		dev->has_len = false;
	}

	PARSE_DEV_OBJ(interrupt) {
		ret = parse_irq(dev, interrupt, index_ident, index);
		if(ret < 0) {
			PARSE_DEV_ERROR(irq);
		}
	}

	PARSE_DEV_OBJ(connect) {
		ret = parse_con(dev, connect, index_ident, index);
		if(ret < 0) {
			PARSE_DEV_ERROR(con);
		}
	}

err_con:
	//json_object_put(connect);
	if(err < 0) {
		for(i = 0; i < dev->irq_num; i++) {
			g_free(dev->irqlist[i].dev);
		}
		g_free(dev->irqlist);
	}
err_irq:
	//json_object_put(interrupt);
err_length:
err_addr:
	//json_object_put(address);
err_class:
	//json_object_put(class);
	*old_list = list;
	return err;
}

static struct devargs* parse_object_list(struct json_object *root,
		struct devargs *list, int *old_n,
		const char *index_ident, int index);

static int parse_group(struct devargs **old_list, int old_n, struct json_object *group)
{
	int n = old_n;
	struct devargs *list = *old_list;
	int i;
	int range[2]; /* range[0] start, range[1] end */
	struct json_object *num;
	struct json_object *obj;
#define PARSE_GROUP_WORD(keyword) \
	do{ \
		obj = json_object_object_get(group, #keyword); \
		if(!obj) { \
			fprintf(stderr, "not found \"%s\" in group.\n", #keyword); \
			goto err_##keyword; \
		} \
	}while(0)

	PARSE_GROUP_WORD(index);
	char *index_ident = g_strdup(json_object_get_string(obj));
	//json_object_put(obj);

	PARSE_GROUP_WORD(range);
	int len = json_object_array_length(obj);
	if(len != 2) {
		fprintf(stderr, "range need 2 integer type argument.");
		goto err_range;
	}
	for(i = 0; i < 2; i++) {
		num = json_object_array_get_idx(obj, i);
		range[i] = json_object_get_int(num);
		//json_object_put(num);
	}
	//json_object_put(obj);
	if(range[0] > range[1]) {
		fprintf(stderr, "range shoud be \"range\": [start, end] and "
				"start <= index < end.\n");
		goto err_range;
	}

	for(i = range[0]; i <= range[1]; i++) {
		PARSE_GROUP_WORD(body);
		list = parse_object_list(obj, list, &n, index_ident, i);
		if(!list) {
			//json_object_put(obj);
			goto err_body;
		}
		//json_object_put(obj);
	}

	*old_list = list;
	return n - old_n;
err_body:
err_range:
	g_free(index_ident);
err_index:
	*old_list = list;
	return -1;
}

static struct devargs* parse_object_list(struct json_object *root,
		struct devargs *list, int *old_n,
		const char *index_ident, int index)
{
	int i, j, ret;
	int n = *old_n;
	struct json_object *class;

	json_object_object_foreach(root, name, object) {
		class = json_object_object_get(object, "class");
		/* group can not exist in group */
		if(class && json_object_get_type(class) == json_type_string &&
				strcmp(json_object_get_string(class), "group") == 0) {
			if(index_ident) {
				fprintf(stderr, "group can't exist in parent group.\n");
				goto err_object;
			}
			ret = parse_group(&list, n, object);
		}else{
			char *tmp_str = deal_str(name, index_ident, index);
			if(!tmp_str) {
				fprintf(stderr, "%s is a error device object name.\n", name);
				goto err_object;
			}
			ret = parse_dev(&list, n, object, tmp_str, index_ident, index);
		}
		//json_object_put(object);
		if(ret < 0) {
			goto err_object;
		}else{
			n += ret;
		}
	}

	*old_n = n;
	return list;

err_object:
	if(*old_n > 0) {
		for(i = *old_n - 1; i < n; i++) {
			g_free(list[i].devclass);
			g_free(list[i].name);
			for(j = 0; j < list[i].irq_num; j++) {
				g_free(list[i].irqlist[j].dev);
			}
			g_free(list[i].irqlist);
			for(j = 0; j < list[i].con_num; j++) {
				g_free(list[i].conlist[j].dev);
				g_free(list[i].conlist[j].con);
			}
			g_free(list[i].conlist);
		}
		/* XXX: ... */
		g_free(list);
	}
	return NULL;
}

static struct devargs* parse_string(const char *str)
{
	struct json_object *root;
	struct devargs *list = NULL;
	int n = 0;

	root = json_tokener_parse(str);
	if(is_error(root)) {
		fprintf(stderr, "json parse failed, and maybe there are some thing wrong\n");
		return NULL;
	}

	list = parse_object_list(root, list, &n, NULL, 0);
	json_object_put(root);
	if(list) {
		list = g_renew(struct devargs, list, n + 1);
		list[n].name = NULL;
		list[n].devclass = NULL;
		return list;
	}else{
		g_free(list);
		return NULL;
	}
}

static struct devargs* parse_conf(const char *path)
{
	char *buf;

	if((buf = file_get_string(path)) == NULL) {
		return NULL;
	}
	return parse_string(buf);
}

#ifdef DYNAMIC_JSON_DEBUG
static void look_devlist(struct devargs *list)
{
	int i, j;
	for(i = 0; list[i].name; i++) {
		printf("dev[%d]: name: %s, class: %s\n", i, list[i].name, list[i].devclass);
		if(list[i].has_addr) {
			printf("\taddress: 0x%lx ", list[i].address);
			if(list[i].has_len)
				printf("length: 0x%lx\n", list[i].length);
			else
				printf("\n");
		}

		if(list[i].irq_num > 0) {
			for(j = 0; j < list[i].irq_num; j++) {
				printf("\tirq[%d]: dev: %s, num: %d\n", j, list[i].irqlist[j].dev,
						list[i].irqlist[j].num);
			}
		}
		if(list[i].con_num > 0) {
			for(j = 0; j < list[i].con_num; j++) {
				printf("\tcon[%d]: dev: %s, con: %s\n", j, list[i].conlist[j].dev,
						list[i].conlist[j].con);
			}
		}
		printf("===========================================\n");
	}
}
#endif

static void free_devlist(struct devargs *list)
{
	int i, j;
	for(i = 0; list[i].name; i++) {
		g_free(list[i].devclass);
		g_free(list[i].name);

		if(list[i].irq_num > 0) {
			for(j = 0; j < list[i].irq_num; j++) {
				g_free(list[i].irqlist[j].dev);
			}
			g_free(list[i].irqlist);
		}
		if(list[i].con_num > 0) {
			for(j = 0; j < list[i].con_num; j++) {
				g_free(list[i].conlist[j].dev);
				g_free(list[i].conlist[j].con);
			}
			g_free(list[i].conlist);
		}
	}
	g_free(list);
}

static int dev_dynamic_create(struct devargs *devlist, void *extra)
{
	int i, ret;
	for(i = 0; devlist[i].name; i++) {
		if((ret = device_create(devlist + i, extra)) < 0)
			return ret;
	}
	return 0;
}

static int dev_dynamic_irq(struct devargs *devlist, void *extra)
{
	int i, ret;
	for(i = 0; devlist[i].name; i++) {
		if((ret = device_irq(devlist + i, extra)) < 0)
			return ret;
	}
	return 0;
}

static int dev_dynamic_con(struct devargs *devlist, void *extra)
{
	int i, ret;
	for(i = 0; devlist[i].name; i++) {
		if((ret = device_con(devlist + i, extra)) < 0)
			return ret;
	}
	return 0;
}

static int dev_dynamic_load(struct devargs *devlist, void *extra)
{
	int ret;
	if((ret = dev_dynamic_create(devlist, extra)) < 0) {
		return ret;
	}
	if((ret = dev_dynamic_con(devlist, extra)) < 0) {
		return ret;
	}
	if((ret = dev_dynamic_irq(devlist, extra)) < 0) {
		return ret;
	}
	return 0;
}

static int dev_dynamic_conf(const char *conf, void *extra)
{
	struct devargs *devlist;
	int ret;

	if(conf == NULL) {
		fprintf(stderr, "conf path is empty.\n");
		return -1;
	}

	devlist = parse_conf(conf);
	if(!devlist) {
		return -1;
	}
#ifdef DYNAMIC_JSON_DEBUG
	look_devlist(devlist);
#endif
	ret = dev_dynamic_load(devlist, extra);
	free_devlist(devlist);
	return ret;
}

int qdev_dynamic_conf(const char *conf, void *extra)
{
	return dev_dynamic_conf(conf, extra);
}
