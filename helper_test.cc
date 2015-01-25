/*
 * All rights reserved.
 *
 * @file         helper_test.cc
 * @author       Arthur Fait
 *
 */

#include <stdio.h>
#include "inotify.hpp"

int main(int argc, char const *argv[])
{
    if (argc < 2) {
        printf("missing argument\n");
        return -1;
    }
    Notifier noti(argv[1], [](Notifier::Event evt){
        printf("path %s, file: %s type: %s(%u) \n", evt.path.c_str(), evt.file.c_str(), Notifier::toString(evt.type), evt.type);
    });
    noti.listen();
    return 0;
}
