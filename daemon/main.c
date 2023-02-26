#include <errno.h>
#include <fcntl.h>
#include <libinput.h>
#include <libudev.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int open_restricted(const char *path, int flags, void *user_data) {
  int fd = open(path, flags);
  return fd < 0 ? -errno : fd;
}

static void close_restricted(int fd, void *user_data) { close(fd); }

const static struct libinput_interface interface = {
    .open_restricted = open_restricted,
    .close_restricted = close_restricted,
};

int main(void) {

  struct libinput *li;
  struct libinput_event *ev;
  struct udev *udev = udev_new();
  struct libinput_device **devices = NULL;
  int dev_count = 0;

  li = libinput_udev_create_context(&interface, NULL, udev);
  libinput_udev_assign_seat(li, "seat0");
  libinput_dispatch(li);

  while ((ev = libinput_get_event(li)) != NULL) {
    if (libinput_event_get_type(ev) == LIBINPUT_EVENT_DEVICE_ADDED) {
        dev_count++;

        // Adding device to device list
        if(devices != NULL)
          devices = (struct libinput_device **)realloc(devices, sizeof(struct libinput_device *) * dev_count);
        else
          devices = (struct libinput_device **)malloc(sizeof(struct libinput_device*));

        // Getting device pointer from event
        struct libinput_device *devptr = libinput_event_get_device(ev);

        devices[dev_count-1] = devptr;
    }

    libinput_event_destroy(ev);
    libinput_dispatch(li);
  }

  printf("Found devices: \n");
  for (int i = 0; i < dev_count; i++) {
    const char* name = libinput_device_get_name(devices[i]);
    printf("%d. %s\n", i+1, name);
  }

  int exit = 0, selected = 0;
  char ch = 0;
  do {
    printf("Select device to show information: ");
    
    if(scanf("%d", &selected) != 1 || (ch = getchar()) != '\n')
    {
      printf("Wrong input. Try again.\n");
      while((ch = getchar()) != '\n'){}
    }
    else if(selected <= 0 || selected > dev_count)
      printf("Device number out of range.\n");
    else exit = 1;

  }while (!exit);

  printf("Selected device: %s.\n", libinput_device_get_name(devices[selected-1]));

  libinput_unref(li);

  return 0;
}
