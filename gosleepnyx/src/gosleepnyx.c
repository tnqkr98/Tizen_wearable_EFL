#include "gosleepnyx.h"
#include <sensor.h>

typedef struct appdata {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *label1;
	Evas_Object *label2;
} appdata_s;

// sensing
Evas_Object *navi;
Evas_Object *start, *stop;
Evas_Object *event_label;
sensor_listener_h listener;

/*------------------------------------------------------- 센싱 콜백 세팅 ---------------------------------------------------- */
void on_sensor_event(sensor_h sensor, sensor_event_s *event, void *user_data){
	sensor_type_e type;
	sensor_get_type(sensor, &type);

	switch(type){
		case SENSOR_HRM:
			dlog_print(DLOG_INFO, LOG_TAG, "%d", event->values[0]);
			char a[100];
			sprintf(a,"f",event->values[0]);
			elm_object_text_set(event_label,a);
			break;
		default:
			dlog_print(DLOG_ERROR, LOG_TAG, "Not an HRM event");
	}
}

void _sensor_accuracy_changed_cb(sensor_h sensor, unsigned long long timestamp, sensor_data_accuracy_e accuracy, void *data){
	dlog_print(DLOG_DEBUG, LOG_TAG, "Sensor accuracy change callback invoked");
}

void _sensor_start_cb(void *data, Evas_object *obj, void *event_info){
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

	// 센서 리스너에 콜백 등록




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
	ad->conform = elm_conformant_add(ad->win);
	elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);		// 화면 위쪽 indicator(상태바) 표시 여부 지정
	elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_OPAQUE);   // indicator 투명도 지정
	evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);    // 오브젝트의 크기를 대략적으로 지정 (EVAS_HINT_EXPAND 는 공간이 허락하는만큼)
	elm_win_resize_object_add(ad->win, ad->conform); // window 객체에 다른 객체를 추가하며 크기를 변경하는 API
	evas_object_show(ad->conform);   // 오브젝트를 화면에 표시. 모든 오브젝트에 공통적으로 사용가능한함수

	/* Label */
	ad->label1 = elm_label_add(ad->conform);
	//elm_object_text_set(ad->label, "<align=center>Hello NYX</align>");
	elm_object_text_set(ad->label1,"HRM");
	//evas_object_size_hint_weight_set(ad->label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);	// 객체의 크기를 지정. 2,3번째가 수평 수직 크기
	//elm_object_content_set(ad->conform, ad->label);		// 위젯의 캡션 택스트를 변경. 위젯?은 conformant 고 그 캡션?텍스트를 label로 지정한건가? conform에 속해서 show 안해도 보임!!
	evas_object_move(ad->label1, 100,100);		// x , y
	evas_object_resize(ad->label1,400,100);    // w , h
	evas_object_show(ad->label1);
	ad->label2 = elm_label_add(ad->conform);
	elm_object_text_set(ad->label2,"ACC");
	evas_object_move(ad->label2, 100,150);		// x , y
	evas_object_resize(ad->label2,400,100);    // w , h
	evas_object_color_set(ad->label2, 255, 0, 0, 255);
	evas_object_show(ad->label2);

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
