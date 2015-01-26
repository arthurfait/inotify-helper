#pragma once

#include <sys/inotify.h>
#include <unistd.h>
#include <limits.h>
#include <atomic>
#include <functional>
#include <thread>

namespace inotify {

enum class EventType: unsigned{
    ACCESS = 0x00000001,
    MODIFY = 0x00000002,
    ATTRIB = 0x00000004,
    CLOSE_WRITE = 0x00000008,
    CLOSE_NOWRITE = 0x00000010,
    OPEN = 0x00000020,
    MOVED_FROM = 0x00000040,
    MOVED_TO = 0x00000080,
    CREATE = 0x00000100,
    DELETE = 0x00000200,
    DELETE_SELF = 0x00000400,
    MOVE_SELF = 0x00000800,
    UNMOUNT = 0x00002000,
    Q_OVERFLOW = 0x00004000,
    IGNORED = 0x00008000,
    // set in event
    DIRECTORY_MODIFIER = 0x40000000
};

inline EventType operator & (EventType a, EventType b) { return EventType(((unsigned)a) & ((unsigned)b)); }
inline EventType operator ^ (EventType a, EventType b) { return EventType(((unsigned)a) ^ ((unsigned)b)); }


static const char* toString(EventType evtType) {
    if((bool)(evtType & EventType::DIRECTORY_MODIFIER)) {
        evtType = evtType^EventType::DIRECTORY_MODIFIER;
    }
    switch(evtType) {
    case EventType::ACCESS:
        return "ACCESS";
        break;
    case EventType::MODIFY:
        return "MODIFY";
        break;
    case EventType::ATTRIB:
        return "ATTRIB";
        break;
    case EventType::CLOSE_WRITE:
        return "CLOSE_WRITE";
        break;
    case EventType::CLOSE_NOWRITE:
        return "CLOSE_NOWRITE";
        break;
    case EventType::OPEN:
        return "OPEN";
        break;
    case EventType::MOVED_FROM:
        return "MOVED_FROM";
        break;
    case EventType::MOVED_TO:
        return "MOVED_TO";
        break;
    case EventType::CREATE:
        return "CREATE";
        break;
    case EventType::DELETE:
        return "DELETE";
        break;
    case EventType::DELETE_SELF:
        return "DELETE_SELF";
        break;
    case EventType::MOVE_SELF:
        return "MOVE_SELF";
        break;
    case EventType::UNMOUNT:
        return "UNMOUNT";
        break;
    case EventType::Q_OVERFLOW:
        return "Q_OVERFLOW";
        break;
    case EventType::IGNORED:
        return "IGNORED";
        break;
    default:
        // unreached, but if reached shit happened
        return "UNKNOWN";
        break;
    }
    return "UNKNOWN";
}


class Notifier {
    std::string clearPath(std::string path) {
        while(!path.empty() && path[path.length() - 1] == '/') {
            path = path.substr(0, path.length() - 1);
        }
        return path;
    }

public:
    static const int DEFAULT_WATCH_FLAGS = (IN_CREATE | IN_DELETE | IN_MODIFY);
    struct Event {
        std::string path;
        std::string file;
        EventType type;
    };
    typedef std::function<void(Event)> FType;

    static const int EVENT_BUF_LEN = 1024 * (sizeof(inotify_event) + NAME_MAX + 1);

    Notifier(std::string path, FType cb, int flags = DEFAULT_WATCH_FLAGS, bool block = true)
        : m_fd(block ? inotify_init() : inotify_init1(IN_NONBLOCK))
        , m_path(clearPath(path))
        , m_callback(cb)
        , m_stop(false)
    {
        inotify_add_watch(m_fd, path.c_str(), flags);
    }

    virtual ~Notifier() {
        if (m_listen_thread.joinable()) {
            m_stop = true;
            m_listen_thread.join();
        }
        close(m_fd);
    }

    void listen() {
        char buffer[EVENT_BUF_LEN];
        int length = read(m_fd, buffer, EVENT_BUF_LEN);
        for (int i = 0; i < length; ++i) {
            struct inotify_event *event = (struct inotify_event*)&buffer[i];
            if (event->len && event->mask != 0) {
                Event e{
                    .path = m_path,
                    .file = std::string(event->name),
                    .type = (EventType)event->mask
                };
                m_callback(e);
            }
            i += sizeof(inotify_event) + event->len;
        }
    }

    void listen_async() {
        m_listen_thread = std::thread([this] {
            while(!m_stop) {
                char buffer[EVENT_BUF_LEN];
                int length = read(m_fd, buffer, EVENT_BUF_LEN);
                for (int i = 0; i < length; ++i) {
                    struct inotify_event *event = (struct inotify_event*)&buffer[i];
                    if (event->len && event->mask != 0) {
                        Event e{
                            .path = m_path,
                            .file = std::string(event->name),
                            .type = (EventType)event->mask
                        };
                        m_callback(e);
                    }
                    i += sizeof(inotify_event) + event->len;
                }
            }
        });
    }


private:
    int m_fd;
    std::string m_path;
    std::thread m_listen_thread;
    std::atomic<bool> m_stop;
    FType m_callback;
};

} // inotify
