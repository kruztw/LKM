#ifndef BROOK_NOTIFIER_H
#define BROOK_NOTIFIER_H

#include <linux/notifier.h>

int register_my_notifier(struct notifier_block *nb);
int unregister_my_notifier(struct notifier_block *nb);

// event type
enum my_msg {
    my_event0,
    my_event1,
    my_event2,
    my_event3
};

#endif
