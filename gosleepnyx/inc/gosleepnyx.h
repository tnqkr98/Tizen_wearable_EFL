#ifndef __gosleepnyx_H__
#define __gosleepnyx_H__

#include <app.h>
#include <Elementary.h>
#include <system_settings.h>
#include <efl_extension.h>
#include <dlog.h>

#define _PRINT_MSG_LOG_BUFFER_SIZE_ 1024
#define PRINT_MSG(fmt, args...) do { char _log_[_PRINT_MSG_LOG_BUFFER_SIZE_]; \
    snprintf(_log_, _PRINT_MSG_LOG_BUFFER_SIZE_, fmt, ##args); _add_entry_text(_log_); } while (0)

void _add_entry_text(const char *text);
Evas_Object *_new_button(appdata_s *ad, Evas_Object *display, char *name, void *cb);
void _create_new_cd_display(appdata_s *ad, char *name, void *cb);
Eina_Bool _pop_cb(void *data, Elm_Object_Item *item);


#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "gosleepnyx"

#if !defined(PACKAGE)
#define PACKAGE "org.example.gosleepnyx"
#endif

#endif /* __gosleepnyx_H__ */
