

#include <string.h>
#include <cstdlib>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
//#include <linux/types.h>
#include <linux/input.h>
#include <fcntl.h>
#include <pthread.h>

#define UINPUT_IOCTL_BASE 'U'
#define UI_DEV_CREATE _IO(UINPUT_IOCTL_BASE, 1)
#define UI_DEV_DESTROY _IO(UINPUT_IOCTL_BASE, 2)
#define UI_SET_EVBIT _IOW(UINPUT_IOCTL_BASE, 100, int)
#define UI_SET_KEYBIT _IOW(UINPUT_IOCTL_BASE, 101, int)
#define UI_SET_RELBIT _IOW(UINPUT_IOCTL_BASE, 102, int)
#define UI_SET_ABSBIT _IOW(UINPUT_IOCTL_BASE, 103, int)
#define UI_SET_MSCBIT _IOW(UINPUT_IOCTL_BASE, 104, int)

#define UINPUT_MAX_NAME_SIZE 80
struct uinput_user_dev {
 char name[UINPUT_MAX_NAME_SIZE];
 struct input_id id;
 __u32 ff_effects_max;
 __s32 absmax[ABS_MAX+1];
 __s32 absmin[ABS_MAX+1];
 __s32 absfuzz[ABS_MAX+1];
 __s32 absflat[ABS_MAX+1];
};

//#define BTN_LEFT 272
//#define BTN_RIGHT 273
//#define BTN_MIDDLE 274
#define KEYCODE_SWITCH_APP 187
#define KEYCODE_HOME 3
#define REL_SCROLL 8

#define SEND_SYN send_event(EV_SYN, EV_SYN, EV_SYN) 
#define SEND_KEY(k, v) send_event(EV_KEY, k, v); SEND_SYN 
#define SEND_LEFT_KEY(v) SEND_KEY(BTN_LEFT, v)
#define SEND_LEFT_KEY_DOWN SEND_LEFT_KEY(1)
#define SEND_LEFT_KEY_UP SEND_LEFT_KEY(0)
#define SEND_RIGHT_KEY(v) SEND_KEY(BTN_RIGHT, v)
#define SEND_RIGHT_KEY_DOWN SEND_RIGHT_KEY(1)
#define SEND_RIGHT_KEY_UP SEND_RIGHT_KEY(0)
#define SEND_MIDDLE_KEY(v) SEND_KEY(BTN_MIDDLE, v)
#define SEND_MIDDLE_KEY_DOWN SEND_MIDDLE_KEY(1)
#define SEND_MIDDLE_KEY_UP SEND_MIDDLE_KEY(0)

#define ACTION_NONE 0
#define ACTION_DOWN 1
#define ACTION_UP 2
#define ACTION_SCROLL 3
#define ACTION_CLICK 4
#define ACTION_DBLCLICK 5
#define ACTION_TOUCH 6
#define ACTION_SWITCH_APP 7
#define ACTION_KEY_SWITCH_APP 8
#define ACTION_HOME 9
#define ACTION_KEY_HOME 10

#define ACTION_EXPIRE_TIME 650
#define ACTION_RESET_TIME 500
#define DOWN_TRIGGER_TIME 800
#define SCROLL_TRIGGER_VALUE 2

int fd, action = ACTION_NONE;
int left_key = 0;
int middle_key = 0;
int right_key = 0;
__s32 scroll, action_x, action_y;

unsigned int action_time = 0;
timeval event_time;
int running = 0;

int moved(__s32 v, __s32 d) {
	return v > d || v < -d; 
}

int moved(__s32 d) {
	return moved(action_x, d) || moved(action_y, d);
}

void send_event(__u16 type, __u16 code, __s32 value)
{
    input_event event;

	event.time = event_time;
    event.type = type;
    event.code = code;
    event.value = value;

    write(fd, &event, sizeof(event));
}

void* run(void *) {
	running = true;
	while (running){
		if (action == ACTION_DOWN && action_time > DOWN_TRIGGER_TIME) {
			action = ACTION_TOUCH;
			SEND_LEFT_KEY_DOWN;
			//printf("down %d\n", action_time);
		}
		if (action == ACTION_KEY_SWITCH_APP || action == ACTION_KEY_HOME) {
		    char command[32];
		    sprintf(command, "input keyevent %d", action == ACTION_KEY_HOME ? KEYCODE_HOME : KEYCODE_SWITCH_APP);
		    system(command);
		    action = ACTION_NONE;
		}
		usleep(40000);
		if (action_time < 40000) action_time += 40;
		//printf("action time %u\n", ms(time));
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
        printf("ver 1.2\n");
		printf("usage: %s /dev/input/event\n", argv[0]);
	    return 1;
	}

	const char * event	= argv[1];
	const char * dev = "/dev/uinput";

	int fe = open(event, O_RDONLY);
	if (fe < 0) {
		printf("%s: %s\n", event, strerror(errno));
	    return 1;
	}

	uinput_user_dev device;
	memset(&device, 0, sizeof(device));

	fd = open(dev, O_WRONLY | O_NONBLOCK);
	if (fd < 0) {
		printf("error %s: %s\n", dev, strerror(errno));
    	return 1;
	}

    strcpy(device.name,"mouse gestures");

    device.id.bustype=BUS_USB;
    device.id.vendor=1;
    device.id.product=1;
    device.id.version=1;

    if (write(fd,&device,sizeof(device)) != sizeof(device)) {
        printf("error setup %s\n", strerror(errno));
		return 1;
    }

	ioctl(fd,UI_SET_EVBIT, EV_SYN);
	
	ioctl(fd,UI_SET_EVBIT, EV_KEY);
	ioctl(fd,UI_SET_KEYBIT, BTN_LEFT);
	ioctl(fd,UI_SET_KEYBIT, BTN_RIGHT);
	ioctl(fd,UI_SET_KEYBIT, BTN_MIDDLE);


    ioctl(fd,UI_SET_EVBIT, EV_REL);
    ioctl(fd,UI_SET_RELBIT, REL_X);
    ioctl(fd,UI_SET_RELBIT, REL_Y);
    ioctl(fd,UI_SET_RELBIT, REL_SCROLL);
	
	ioctl(fd,UI_SET_EVBIT, EV_MSC);
	ioctl(fd,UI_SET_MSCBIT, EV_MSC);
	
    if (ioctl(fd,UI_DEV_CREATE) < 0) {
        printf("error UI_DEV_CREATE %s\n", strerror(errno));
		return 1;
    }

	pthread_t t;
 	pthread_create(&t, NULL, run, NULL);
 
	ioctl(fe, EVIOCGRAB, 1);

	input_event e;
	while(read(fe, &e, sizeof(input_event)) > 0) {
		event_time = e.time;
		if (e.code != REL_SCROLL || e.type != EV_REL) {
			write(fd, &e, sizeof(input_event));
			if (e.type == EV_KEY) {
				if (e.code == BTN_LEFT) left_key = e.value;
				if (e.code == BTN_MIDDLE) middle_key = e.value;
				if (e.code == BTN_RIGHT) right_key = e.value;
				if (middle_key && right_key) break;
			}
			else if (e.type == EV_REL) {
				if (e.code == REL_X) action_x += e.value;
				if (e.code == REL_Y) action_y += e.value;
				if (action == ACTION_DOWN) {
					if (moved(2)) {
						action = ACTION_TOUCH;
						SEND_LEFT_KEY_DOWN;
					}
				}
				else if (action == ACTION_UP) {
					if (moved(action_y, 4)) {
                        action = ACTION_NONE;
                        action_time = ACTION_RESET_TIME;
                    }
					else if (moved(action_x, 2)) {
					    action = action_x > 0 ? ACTION_SWITCH_APP : ACTION_HOME;
					}
				}
				else if (action == ACTION_SWITCH_APP || action == ACTION_HOME) {
					if (moved(action_x, 32) || moved(action_y, 4)) {
					    action = ACTION_NONE;
						action_time = ACTION_RESET_TIME;
					}
					else if (action == ACTION_SWITCH_APP) {
					    if (e.code == REL_X && e.value < 0) {
					        action = ACTION_KEY_SWITCH_APP;
					    }
					}
					else if (action == ACTION_HOME) {
					    if (e.code == REL_X && e.value > 0) {
					        action = ACTION_KEY_HOME;
					    }
					}
				}
			}
		} else {
			__s32 s = e.value;
			
			
			int a = s > 0 ? ACTION_DOWN : ACTION_UP;
			
			if (action_time > ACTION_EXPIRE_TIME && action != ACTION_TOUCH) {
				action = a;
				action_time = 0;
				action_x = action_y = 0;
				scroll = s;
				//printf("action %d %u\n", action, action_time);
			}
			else if (action == ACTION_SCROLL) {
				//printf("s/z %u\n", action_time);
				action_time = 0;
				write(fd, &e, sizeof(input_event));
			}
			else if (a == action) {
				scroll += s;
				if (scroll > SCROLL_TRIGGER_VALUE || scroll < -SCROLL_TRIGGER_VALUE) {
					//printf("scroll time %u scroll %d\n", action_time, scroll);
					action = ACTION_SCROLL;
					//e.value = scroll;
					write(fd, &e, sizeof(input_event));
				}
				action_time = 0;
			} 
			else if (a == ACTION_UP && action == ACTION_TOUCH) {
				action = ACTION_NONE;
				action_time = ACTION_RESET_TIME;
				SEND_LEFT_KEY_UP;
				//printf("up\n");
			}
			else if (a == ACTION_UP && action == ACTION_DOWN) {
				action = ACTION_CLICK;
				SEND_LEFT_KEY_DOWN;
				SEND_LEFT_KEY_UP;
				//printf("click\n");
			}
			else if (a == ACTION_DOWN && action == ACTION_CLICK) {
				action = ACTION_DBLCLICK;
				action_time = ACTION_RESET_TIME;
				SEND_LEFT_KEY_DOWN;
				SEND_LEFT_KEY_UP;
				//printf("dblclick\n");
			}
			else if (a == ACTION_DOWN && action == ACTION_UP) {
				action = ACTION_NONE;
				action_time = ACTION_RESET_TIME;
				SEND_RIGHT_KEY_DOWN;
				SEND_RIGHT_KEY_UP;
				//printf("back\n");
			}
			//printf("%u type=%d,code=%d,value=%d\n", ms(event_time), e.type, e.code, e.value);
		}
	}

	running = false;
	usleep(50000);

	ioctl(fe, EVIOCGRAB, 0);
	close(fe);
	
	ioctl(fd,UI_DEV_DESTROY);
	close(fd);
}
