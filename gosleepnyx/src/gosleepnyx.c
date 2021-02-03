#include "gosleepnyx.h"
#define BUFLEN 200

// sensing
Evas_Object *GLOBAL_DEBUG_BOX;
Evas_Object *conform;
Evas_Object *start, *stop;
Evas_Object *event_label;
sensor_listener_h listener;

/*------------------------------------------------------- 센싱 콜백 세팅 ---------------------------------------------------- */
void on_sensor_event(sensor_h sensor, sensor_event_s *event, void *user_data){
	sensor_type_e type;
	sensor_get_type(sensor, &type);

	switch(type){
		case SENSOR_HRM:
			dlog_print(DLOG_INFO, LOG_TAG, "sensor_hrm : %f", event->values[0]);
			char a[100];
			sprintf(a,"%f",event->values[0]);
			elm_object_text_set(event_label,a);
			break;
		default:
			dlog_print(DLOG_ERROR, LOG_TAG, "Not an HRM event");
	}
}

void _sensor_accuracy_changed_cb(sensor_h sensor, unsigned long long timestamp, sensor_data_accuracy_e accuracy, void *data){
	dlog_print(DLOG_DEBUG, LOG_TAG, "Sensor accuracy change callback invoked");
}

void _sensor_start_cb(void *data, Evas_Object *obj, void *event_info){
	void *user_data = NULL;
	char out[100];

	// Retrieving a Sensor		 HRM 센서가 있는지 확인  ------------------------------------------------------
	sensor_type_e type = SENSOR_HRM;
	sensor_h sensor;

	bool supported;
	int error = sensor_is_supported(type, &supported);
	if(error != SENSOR_ERROR_NONE){
		dlog_print(DLOG_ERROR, LOG_TAG, "sensor is supported error : %d",error);
		return;
	}
	
	//Get sensor list ? 왜 ? 한 보드에 HRM 센서가 여러개? 있을수있는 건가? (red, green 빛기반?) ----------------
	int count;
	sensor_h *list;
	error = sensor_get_sensor_list(type, &list, &count);
	if(error != SENSOR_ERROR_NONE)
		dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_sensor_list error : %d",error);
	else{
		dlog_print(DLOG_DEBUG,LOG_TAG, "Number of sensors: %d",count);
		free(list);
	}

	error = sensor_get_default_sensor(type, &sensor);
	if(error != SENSOR_ERROR_NONE){
		dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_default_sensor error : %d", error);
		return;		// 기본 센서도 없으면 아예 종료
	}
	dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_get_default_sensor");	// 기본 센서 참조획득

	// 센서 리스너 등록, min_interval 값 획득  --------------------------------------------------------------------------------
	error = sensor_create_listener(sensor, &listener);
	if(error != SENSOR_ERROR_NONE){
		dlog_print(DLOG_ERROR, LOG_TAG, "sensor_create_listener_error : %d", error);
		return;
	}
	dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_get_default_sensor");	// 기본 센서 참조획득

	int min_interval = 0;
	error = sensor_get_min_interval(sensor, &min_interval);
	if(error != SENSOR_ERROR_NONE){
		dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_min_interval_error : %d", error);
		return;
	}
	dlog_print(DLOG_DEBUG, LOG_TAG, "Minimal interval of the sensor : %d", min_interval);	// Minmal interval  값

	// 센서 리스너에 콜백(센서의 값변화 이벤트) 등록
	error = sensor_listener_set_event_cb(listener, min_interval, on_sensor_event, user_data);
	if(error != SENSOR_ERROR_NONE){
		dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_set_event_cb error : %d",error);
		return;
	}
	dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_listener_set_event_cb");

	// 리스너에 Accuracy Changed 콜백 등록
	error = sensor_listener_set_accuracy_cb(listener, _sensor_accuracy_changed_cb, user_data);
	if(error != SENSOR_ERROR_NONE){
		dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_set_accuracy_cb error : %d", error);
		return;
	}
	dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_listener_set_accuracy_cb");

	// 리스너에 interval 값 지정
	error = sensor_listener_set_interval(listener,100);
	if(error != SENSOR_ERROR_NONE){
		dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_set_interval error : %d", error);
		return;
	}
	dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_listener_set_interval");

	// 리스너에 option 지정
	error = sensor_listener_set_option(listener,SENSOR_OPTION_ALWAYS_ON);
	if(error != SENSOR_ERROR_NONE){
		dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_set_option error : %d", error);
		return;
	}
	dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_listener_set_option");

	// 리스너 시작
	error = sensor_listener_start(listener);
	if(error != SENSOR_ERROR_NONE){
		dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_start error : %d", error);
		return;
	}
	dlog_print(DLOG_DEBUG, LOG_TAG, "sensor_listener_start");

	// 센싱
	sensor_event_s event;
	error = sensor_listener_read_data(listener, &event);
	if(error != SENSOR_ERROR_NONE){
		dlog_print(DLOG_DEBUG,LOG_TAG,"sensor_listener_read_data error : %d",error);
		return;
	}

	// 센싱 데이터 화면 출력
	switch(type){
		case SENSOR_HRM:
			dlog_print(DLOG_INFO, LOG_TAG, "sensor_hrm : %f", event.values[0]);
			sprintf(out,"%f",event.values[0]);
			break;
		default:
			dlog_print(DLOG_ERROR,LOG_TAG,"Not an HRM event!");
	}
	dlog_print(DLOG_DEBUG, LOG_TAG, out);

	char *name = NULL;
    char *vendor = NULL;
    float min_range = 0.0;
    float max_range = 220.0;
    float resolution = 0.0;

    error = sensor_get_name(sensor, &name);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_name error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "Sensor name: %s", name);
    free(name);

    error = sensor_get_vendor(sensor, &vendor);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_vendor error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "Sensor vendor: %s", vendor);
    free(vendor);

    error = sensor_get_type(sensor, &type);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_type error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "Sensor type: %s",
            type == SENSOR_ACCELEROMETER               ? "Accelerometer"
          : type == SENSOR_GRAVITY                     ? "Gravity sensor"
          : type == SENSOR_LINEAR_ACCELERATION         ? "Linear acceleration sensor"
          : type == SENSOR_MAGNETIC                    ? "Magnetic sensor"
          : type == SENSOR_ROTATION_VECTOR             ? "Rotation Vector sensor"
          : type == SENSOR_ORIENTATION                 ? "Orientation sensor"
          : type == SENSOR_GYROSCOPE                   ? "Gyroscope sensor"
          : type == SENSOR_LIGHT                       ? "Light sensor"
          : type == SENSOR_PROXIMITY                   ? "Proximity sensor"
          : type == SENSOR_PRESSURE                    ? "Pressure sensor"
          : type == SENSOR_ULTRAVIOLET                 ? "Ultraviolet sensor"
          : type == SENSOR_TEMPERATURE                 ? "Temperature sensor"
          : type == SENSOR_HUMIDITY                    ? "Humidity sensor"
          : type == SENSOR_HRM                         ? "Heart Rate Monitor sensor (Since Tizen 2.3.1)"
          : type == SENSOR_HRM_LED_GREEN               ? "HRM (LED Green) sensor (Since Tizen 2.3.1)"
          : type == SENSOR_HRM_LED_IR                  ? "HRM (LED IR) sensor (Since Tizen 2.3.1)"
          : type == SENSOR_HRM_LED_RED                 ? "HRM (LED RED) sensor (Since Tizen 2.3.1)"
          : type == SENSOR_LAST                        ? "End of sensor enum values" : "Custom sensor");

    error = sensor_get_min_range(sensor, &min_range);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_min_range error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "Minimum range of the sensor: %f", min_range);

    error = sensor_get_max_range(sensor, &max_range);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_max_range error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "Maximum range of the sensor: %f", max_range);

    error = sensor_get_resolution(sensor, &resolution);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_get_resolution error: %d", error);
        return;
    }
    dlog_print(DLOG_DEBUG, LOG_TAG, "Resolution of the sensor: %f", resolution);

    elm_object_disabled_set(start, EINA_TRUE);
    elm_object_disabled_set(stop, EINA_FALSE);
}


void _sensor_stop_cb(void *data, Evas_Object *obj, void *event_info)
{
    int error = sensor_listener_unset_event_cb(listener);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_unset_event_cb error: %d", error);
    }

    error = sensor_listener_stop(listener);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_listener_stop error: %d", error);
    }

    error = sensor_destroy_listener(listener);
    if (error != SENSOR_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "sensor_destroy_listener error: %d", error);
    }

    elm_object_disabled_set(start, EINA_FALSE);
    elm_object_disabled_set(stop, EINA_TRUE);
}

void _add_entry_text(const char *text)      // ???
{
    Evas_Coord c_y;
    elm_entry_entry_append(GLOBAL_DEBUG_BOX, text);
    elm_entry_entry_append(GLOBAL_DEBUG_BOX, "<br>");
    elm_entry_cursor_end_set(GLOBAL_DEBUG_BOX);
    elm_entry_cursor_geometry_get(GLOBAL_DEBUG_BOX, NULL, &c_y, NULL, NULL);
    elm_scroller_region_show(GLOBAL_DEBUG_BOX, 0, c_y, 0, 0);
}

Eina_Bool _pop_cb(void *data, Elm_Object_Item *item){
    elm_win_lower(((appdata_s *)data)->win);
    return EINA_FALSE;
}

Evas_Object *_new_button(appdata_s *ad, Evas_Object *display, char *name, void *cb){
    // Create a button
    Evas_Object *bt = elm_button_add(display);
    elm_object_text_set(bt, name);
    evas_object_smart_callback_add(bt, "clicked", (Evas_Smart_Cb) cb, ad);
    evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_box_pack_end(display, bt);
    evas_object_show(bt);
    return bt;
}

void _create_new_cd_display(appdata_s *ad, char *name, void *cb){
    // Create main box
    Evas_Object *box = elm_box_add(conform);
    elm_object_content_set(conform, box);
    elm_box_horizontal_set(box, EINA_FALSE);
    evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_show(box);

    start = _new_button(ad, box, "Start", _sensor_start_cb);

    event_label = elm_label_add(box);
    elm_object_text_set(event_label, "NYX HRM!");
    elm_box_pack_end(box, event_label);
    evas_object_show(event_label);

    stop = _new_button(ad, box, "Stop", _sensor_stop_cb);
}

/*------------------------------------------------------  기본 시스템 세팅 -------------------------------------------------- */

// 앱 삭제 요청 발생 시 실행되는 이벤트 함수
static void win_delete_request_cb(void *data, Evas_Object *obj, void *event_info) {ui_app_exit();}

static void win_back_cb(void *data, Evas_Object *obj, void *event_info)		// Back 버튼을 눌렀을 때 실행되는 이벤트 함수
{
	appdata_s *ad = data;
	/* Let window go to hide state. */
	elm_win_lower(ad->win);
}

static void create_base_gui(appdata_s *ad)		// 화면을 구성하는 윈도우와 각종 컨테이너. 위젯 생성 함수
{
	/* Window */
	// elm_win 을 초기화 한다, 화면을 조작하려면 필수다 이건.
	ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE); 		  // Window(화면 레이아웃 최상위 객체) 객체를 생성하는 API.
	elm_win_autodel_set(ad->win, EINA_TRUE);
	elm_win_conformant_set(ad->win,EINA_TRUE);
	elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);		// 화면 위쪽 indicator(상태바) 표시 여부 지정
	elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_OPAQUE);   // indicator 투명도 지정

	if (elm_win_wm_rotation_supported_get(ad->win)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(ad->win, (const int *)(&rots), 4);	// 화면이 모든 회전 각도를 지원하도록 함.
	}

	// 스마트 오브젝트(위젯,컨테이너 등)에 이벤트 콜백함수 지정. " delete, request" 이벤트 팔생시 , win_delete_request_cb 콜백
	evas_object_smart_callback_add(ad->win, "delete,request", win_delete_request_cb, NULL);
	// 오브젝트에 이벤트 콜백함수 지정. API
	eext_object_event_callback_add(ad->win, EEXT_CALLBACK_BACK, win_back_cb, ad);

	/* Conformant */
	/*  conformant 는 화면에 새 영역이 추가 될때 (ex. 키패드) 윈도우 크기를 변경해줌. 하나의 앱은 하나의 conformant 만을 가져야함*/
	conform = elm_conformant_add(ad->win);
	evas_object_size_hint_weight_set(conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);    // 오브젝트의 크기를 대략적으로 지정 (EVAS_HINT_EXPAND 는 공간이 허락하는만큼)
	elm_win_resize_object_add(ad->win, conform); // window 객체에 다른 객체를 추가하며 크기를 변경하는 API
	evas_object_show(conform);   // 오브젝트를 화면에 표시.

	/* Create a naviframe */
	ad->navi = elm_naviframe_add(conform);
	evas_object_size_hint_align_set(ad->navi, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(ad->navi, EVAS_HINT_EXPAND,EVAS_HINT_EXPAND);

	elm_object_content_set(conform, ad->navi);
	evas_object_show(ad->navi);

	// 메인윈도우에 버튼생성
	_create_new_cd_display(ad, "Sensor", _pop_cb);
	eext_object_event_callback_add(ad->navi, EEXT_CALLBACK_BACK, eext_naviframe_back_cb,NULL);


	/* Label */
	/*ad->label1 = elm_label_add(ad->conform);
	//elm_object_text_set(ad->label, "<align=center>Hello NYX</align>");
	elm_object_text_set(ad->label1,"HRM");
	//evas_object_size_hint_weight_set(ad->label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);	// 객체의 크기를 지정. 2,3번째가 수평 수직 크기
	//elm_object_content_set(ad->conform, ad->label);		// 위젯의 캡션 택스트를 변경. 위젯?은 conformant 고 그 캡션?텍스트를 label로 지정한건가? conform에 속해서 show 안해도 보임!!
	evas_object_move(ad->label1, 100,100);		// x , y
	evas_object_resize(ad->label1,400,100);    // w , h
	//evas_object_show(ad->label1);
	ad->label2 = elm_label_add(ad->conform);
	elm_object_text_set(ad->label2,"ACC");
	evas_object_move(ad->label2, 100,150);		// x , y
	evas_object_resize(ad->label2,400,100);    // w , h
	evas_object_color_set(ad->label2, 255, 0, 0, 255);
	//evas_object_show(ad->label2);*/

	/* Show window after base gui is set up */
	evas_object_show(ad->win);
}

static bool app_create(void *data){
	appdata_s *ad = data;
	create_base_gui(ad);

	return true;
}

static void app_control(app_control_h app_control, void *data){/* Handle the launch request. */}
static void app_pause(void *data){/* Take necessary actions when application becomes invisible. */}
static void app_resume(void *data){/* Take necessary actions when application becomes visible. */}
static void app_terminate(void *data){/* Release all resources. */}

static void ui_app_lang_changed(app_event_info_h event_info, void *user_data){
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
	elm_language_set(locale);
	free(locale);
	return;
}

static void ui_app_orient_changed(app_event_info_h event_info, void *user_data){/*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/return;}
static void ui_app_region_changed(app_event_info_h event_info, void *user_data){/*APP_EVENT_REGION_FORMAT_CHANGED*/}
static void ui_app_low_battery(app_event_info_h event_info, void *user_data){/*APP_EVENT_LOW_BATTERY*/}
static void ui_app_low_memory(app_event_info_h event_info, void *user_data){/*APP_EVENT_LOW_MEMORY*/}

int main(int argc, char *argv[])
{
	appdata_s ad = {0,};
	int ret = 0;

	ui_app_lifecycle_callback_s event_callback = {0,};
	app_event_handler_h handlers[5] = {NULL, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, ui_app_low_battery, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, ui_app_low_memory, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &ad);

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);
	}

	return ret;
}
