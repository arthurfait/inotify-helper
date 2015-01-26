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
    inotify::Notifier noti(argv[1], [](inotify::Notifier::Event evt){
        printf("path %s, file: %s type: %s(%x) \n", evt.path.c_str(), evt.file.c_str(), inotify::toString(evt.type), evt.type);
    });
    noti.listen_async();
    sleep(100);
    return 0;
}
