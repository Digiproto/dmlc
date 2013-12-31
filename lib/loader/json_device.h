#ifndef __JSON_DEVICE_H__
#define __JSON_DEVICE_H__

#ifndef __cplusplus
#ifndef bool
typedef enum bool {
	false,
	true,
} bool;
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* following structure is temporary */
struct irq_intf {
	char *dev;
	int   num;
};

struct con_intf {
	char *con;
	int   index;
	char *dev;
	char *port;
};

struct attr_intf {
	char *attr;
	int   index;
	char *type;
	char *value;
};

struct devargs {
	char             *devclass;
	char             *name;
	bool              has_addr;
	unsigned long     address;
	bool              has_len;
	unsigned long     length;
	int               irq_num;
	struct irq_intf  *irqlist;
	int               con_num;
	struct con_intf  *conlist;
	int               attr_num;
	struct attr_intf *attrlist;
};

int device_create(struct devargs *dev, void *extra);
int device_con(struct devargs *dev, void *extra);
int device_irq(struct devargs *dev, void *extra);
int device_attr(struct devargs *dev, void *extra);

#ifdef __cplusplus
}
#endif

#endif
