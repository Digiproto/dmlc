#include <stdio.h>

#include <simics/dev_iface.h>
#include <simics/base_types.h>
#include <simics/bank_access.h>
#include <simics/core/object_model.h>
#include <simics/core/object_class.h>
#include <simics/core/object_iface.h>
#include <simics/core/object_resource.h>

//#include "skyeye_sched.h"
#include "s3c6410x_timer.h"

//static int s3c6410x_scheduler_id = -1;
//static void s3c6410x_timer_callback(void *obj)
static void* s3c6410x_timer_callback(void *obj)
{
	s3c6410x_timer_t *timer_dev = (s3c6410x_timer_t *)obj;

	if ((timer_dev->regs.tcon & 0x100000) != 0) {
		/*tcntx is the orignal value we set, it equals tcntbx firstly*/
		//io.timer.tcnt[4] = io.timer.tcnt[4] - 100;
		timer_dev->regs.tcnt[4] -= ((timer_dev->regs.tcntb[4] / 10) + 1);

		if (timer_dev->regs.tcnt[4] <= 0) {
			/* whe the tcntx is 0, reset the timer tcntx as the value of
			 * tcntb
			 */
			timer_dev->regs.tcnt[4] = timer_dev->regs.tcntb[4];
			/*timer 4 hasn't tcmp */
			timer_dev->regs.tcnto[4] = timer_dev->regs.tcntb[4];
			/* Timer4 request status*/

			/* set timer4 interrupt */
			timer_dev->irq.iface->signal_raise(timer_dev->irq.obj);
		}
	}
	return NULL;
}

static exception_type_t
s3c6410x_timer_write (conf_object_t *obj, physical_address_t offset, const void *buf,  size_t count)
{
	s3c6410x_timer_t *timer_dev = (s3c6410x_timer_t *)obj;
	uint32 data = *(uint32 *)buf;

	switch (offset) {
	case TCFG0:
		timer_dev->regs.tcfg0 = data;
		break;
	case TCFG1:
		timer_dev->regs.tcfg1 = data;
		break;
	case TCON:
	{
		timer_dev->regs.tcon = data;

		/* 2010-07-27 added by Jeff.Du. Used timer scheduler */
		/* timer4 */
		/* timer4  update*/
		if ((timer_dev->regs.tcon & 0x200000) != 0) {

			/* if timer4 is started */
			//if(s3c6410x_scheduler_id != -1 ){
			if(schedule_tmp_status_sched(timer_dev->sched) == SchedTmpRun){
				/* prescaler for timer4 */
				int scaler = ((timer_dev->regs.tcfg0 & 0xff00) >> 8);
				/* divider selection for timer4 frequency */
				int div = ((timer_dev->regs.tcfg1 & 0xf0000) >> 16);

				/* get the divider */
				int mux = 1;		/* divider for timer4 */
				switch(div){
					case 0x0:
						mux=1;
						break;
					case 0x1:
						mux=2;
						break;
					case 0x2:
						mux=4;
						break;
					case 0x3:
						mux=8;
						break;
					case 0x4:
						mux=16;
						break;
					default:
						mux=1;
						break;
				}

				/* timer4 frequency */
				long long freq = ((66500000/(scaler + 1))/mux);
				/* get timer4 occur time */
				int us = (int)(timer_dev->regs.tcntb[4] / (freq));
				/* get timer4 mode */
				//int mode = (timer_dev->regs.tcon & 0x400000)?Periodic_sched:Oneshot_sched;
				int mode = (timer_dev->regs.tcon & 0x400000)?SchedTmpLoop:SchedTmpOnce;
				/* check if a proper value */
				if (us == 0 && timer_dev->regs.tcntb[4])
					us = 1000;
				/* update timer4 */
				//mod_thread_scheduler(s3c6410x_scheduler_id, (unsigned int)us, mode);
				schedule_tmp_update(timer_dev->sched, s3c6410x_timer_callback, (void*)obj, mode, us);
			}
		}

		/* timer4 start or stop */
		if ((timer_dev->regs.tcon & 0x100000) != 0) {
			/* set internal timer */
			//if(s3c6410x_scheduler_id == -1 ){
			if(schedule_tmp_status_sched(timer_dev->sched) != SchedTmpRun){
				/* prescaler for timer4 */
				int scaler = ((timer_dev->regs.tcfg0 & 0xffff00) >> 8);
				/* divider selection for timer4 frequency */
				int div = ((timer_dev->regs.tcfg1 & 0x30000) >> 16);

				int mux = 1;		/* divider for timer4 */
				/* get the divider */
				switch(div){
					case 0x0:  /* 1/2 */
						mux=1;
						break;
					case 0x1:  /* 1/4 */
						mux=2;
						break;
					case 0x2:  /* 1/8 */
						mux=4;
						break;
					case 0x3:  /* 1/16 */
						mux=8;
						break;
					case 0x4:  /* Extern FCLK0 */
						mux=16;
						break;
					default:
						mux=1;
						break;
				}

				/* timer4 frequency */
				long long freq = ((66500000/(scaler + 1))/mux);
				/* get timer4 occur time */
				int us = (int)(timer_dev->regs.tcntb[4] / (freq));
				//printf("[skyeye] tcntb[4] = %d, freq = %lld\n", timer_dev->regs.tcntb[4], freq);
				/* get timer4 mode */
				//int mode = (timer_dev->regs.tcon & 0x400000)?Periodic_sched:Oneshot_sched;
				int mode = (timer_dev->regs.tcon & 0x400000)?SchedTmpLoop:SchedTmpOnce;
				/* check if a proper value */
				if (us == 0 && timer_dev->regs.tcntb[4])
					us = 1000;
				/* create a timer scheduler */
				//create_thread_scheduler(us, mode, s3c6410x_timer_callback,
				//		(void*)obj, &s3c6410x_scheduler_id);
				timer_dev->sched = schedule_tmp_insert(timer_dev->group, s3c6410x_timer_callback,
						(void*)obj, mode, us);
			}
		} else {
			//if (s3c6410x_scheduler_id != -1) {
			if(schedule_tmp_status_sched(timer_dev->sched) == SchedTmpRun){
				//del_thread_scheduler(s3c6410x_scheduler_id);
				//s3c6410x_scheduler_id = -1;
				schedule_tmp_delete(timer_dev->sched);
			}
		}
		/* timer4 end */
		break;
	}
	case TCNTB0:
	case TCNTB1:
	case TCNTB2:
	case TCNTB3:
	case TCNTB4:
	{
		int n = (offset - 0xC) / 0xC;
		//printf("write tcntb4[%d] %d\n", n, data);
		timer_dev->regs.tcntb[n] = data;
		#if 0
		/* io.timer.tcntb[n] = data; */
		/* temp data taken from linux source */
		io.timer.tcntb[n] = 25350 / 20;
		#endif
		break;
	}
	case TCMPB0:
	case TCMPB1:
#if 0
	case TCMPB2:
	case TCMPB3:
#endif
	{
		int n = (offset - 0x10) / 0xC;
		timer_dev->regs.tcmpb[n] = data;
		break;
	}
	/* shenoubang 2012-4-19 */
	case TCNTO0:
	case TCNTO1:
	case TCNTO2:
	case TCNTO3:
	{
		int n = (offset - 0x14) / 0xc;
		timer_dev->regs.tcnto[n] = data;
		break;
	}
	case TCNTO4:
		timer_dev->regs.tcnto[4] = data;
		break;
	case TINT_CSTAT:
		timer_dev->regs.tint_cstat = data;
		break;
	default:
		break;
	}
	return No_exp;
}

static exception_type_t
s3c6410x_timer_read (conf_object_t *obj, physical_address_t offset, void *buf, size_t count)
{
	s3c6410x_timer_t *timer_dev = (s3c6410x_timer_t*) obj;
	uint32_t data;
	switch (offset) {
	case TCFG0:
		data = timer_dev->regs.tcfg0;
		break;
	case TCFG1:
		data = timer_dev->regs.tcfg1;
		break;
	case TCON:
		data = timer_dev->regs.tcon;
		break;
	case TCNTB0:
	case TCNTB1:
	case TCNTB2:
	case TCNTB3:
	case TCNTB4:
	{
		int n = (offset - 0xC) / 0xC;
		data = timer_dev->regs.tcntb[n];
		break;
	}
	case TCMPB0:
	case TCMPB1:
	{
		int n = (offset - 0x10) / 0xC;
		data = timer_dev->regs.tcmpb[n];
		break;
	}
	case TCNTO0:
	case TCNTO1:
	case TCNTO2:
	case TCNTO3:
	{
		int n = (offset - 0x10) / 0xC;
		data = timer_dev->regs.tcnto[n];
		break;
	}
	case TCNTO4:
		data = timer_dev->regs.tcnt[4];
		break;
	/* shenoubang 2012-4-19 */
	case TINT_CSTAT:
		data = timer_dev->regs.tint_cstat;
		break;
	default:
		break;
	}
	*(uint32_t *)buf = data;
	//printf("timer_read: addr 0x%x, val 0x%x\n", offset, *((int*) buf));
	return No_exp;
}

static int signal_set(void *_, conf_object_t *obj, attr_value_t *val, attr_value_t *_index) {
    s3c6410x_timer_t *_dev = (s3c6410x_timer_t *)obj;
	void const *iface = NULL;
	conf_object_t *peer = NULL;
	const char *port = NULL;

	if(SIM_attr_is_object(*val)) {
		peer = SIM_attr_object(*val);
	}else if(SIM_attr_is_list(*val)) {
		peer = SIM_attr_object(SIM_attr_list_item(*val, 0));
		port = SIM_attr_string(SIM_attr_list_item(*val, 1));
	}
    if(_dev->irq.obj == peer && _dev->irq.port == port) {
        return 0;
    }
    _dev->irq.obj = peer;
    if(_dev->irq.port)
        MM_FREE((void *)_dev->irq.port);
    if(port) {
        _dev->irq.port = MM_STRDUP(port);
    } else {
        _dev->irq.port = NULL;
    }
    if(port) {
        iface = SIM_get_port_interface(peer, "signal", port);
    } else {
        iface = SIM_get_interface(peer, "signal");
    }
    _dev->irq.iface = iface;
    return No_exp;
}

static attr_value_t signal_get(void *_, conf_object_t *obj, attr_value_t *_index) {
    s3c6410x_timer_t *_dev = (s3c6410x_timer_t *)obj;
	attr_value_t attr;
	if(_dev->irq.port) {
		attr = SIM_alloc_attr_list(2);
		SIM_attr_list_set_item(&attr, 0, SIM_make_attr_string(_dev->irq.port));
		SIM_attr_list_set_item(&attr, 0, SIM_make_attr_obj(_dev->irq.obj));
	}
	return attr;
}

static const struct ConnectDescription timer_connects[] = {
	[0] = {
		.name = "irq",
		.set = signal_set,
		.get = signal_get,
	},
	[1] = {}
};

static const struct InterfaceDescription timer_ifaces[] =  {
	[0] = {}
};

static const struct AttributeDescription timer_attributes[] = {
	[0] = {}
};

static const class_resource_t timer_resource = {
	.ifaces = timer_ifaces,
	.connects = timer_connects,
	.attributes = timer_attributes,
};

static const bank_access_t regs_access = {
	.read = s3c6410x_timer_read,
	.write = s3c6410x_timer_write,
};

static const struct bank_access_description timer_bank_access[] = {
	[0] = (struct bank_access_description) {
		.name = "regs",
		.access = &regs_access,
	},
	[1] = {}
};

static conf_object_t* timer_new_instance(const char *objname) {
	s3c6410x_timer_t *timer = (s3c6410x_timer_t*) MM_ZALLOC(sizeof(*timer));
	timer->group = schedule_tmp_create();
	SIM_object_register(&timer->obj, objname);
	return &timer->obj;
}

static const class_data_t timer_data = {
	.new_instance = timer_new_instance,
	.resources = &timer_resource,
	.bank_access = timer_bank_access,
};

void init_local(void) {
	static conf_class_t conf_class = {
		.cls_name = "s3c6410x_timer",
		.cls_data = &timer_data,
	};
	SIM_register_conf_class(conf_class.cls_name, &conf_class);
}
