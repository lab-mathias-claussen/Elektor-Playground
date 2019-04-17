#include <lvgl.h>
#include <TFT_eSPI.h> 
#include <Ticker.h>

/**********************************
 *      DEFINES
 **********************************/
#define LVGL_TICK_PERIOD 20
/**********************************
 *  GLOBAL VARIABLES / OBJECTS
 **********************************/
TFT_eSPI tft = TFT_eSPI();      /* generate tft instance */ 
Ticker   lvgl_tick;             /* timer for lvgl */

/**************************************************************************************************
 *    Function      : disp_flush_data
 *    Description   : Sends pixels to the display
 *    Input         : int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t *color_array 
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void disp_flush_data(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t *color_array)
{
  /* We set swapped bytes to ture, otherwyse the color get wrong */
  tft.setSwapBytes(true);
  tft.pushImage(x1, y1,(x2 - x1 + 1), (y2 - y1 + 1), (uint16_t*) color_array);  
  /*Tell the flushing is ready*/
  lv_flush_ready();
  
}

/**************************************************************************************************
 *    Function      : x2064_tp_read
 *    Description   : reads the actual status of the touchscreen
 *    Input         : lv_indev_data_t *data 
 *    Output        : none
 *    Remarks       : We don't use the TP_IRQ pin 
 **************************************************************************************************/
bool x2064_tp_read(lv_indev_data_t *data)
{
  /* we store the last x and y coodinates here */
  static uint16_t last_x=0;
  static uint16_t last_y=0;
  
  uint16_t x=4096;
  uint16_t y=4096;
  /* 
   *  if the dirver would requiere a second call to get all data 
   *  this must be as long true as we need to collet it. As we only
   *  have a touchscreen this gets pretty simple
   */
  bool moredata = false; /* We get all data in once call */
  /*Check if we have given not a NULL-pointer */
  assert(data);
  /* 
   *  This collects the raw data from the screen 
   *  to save one pin we ignore the TP_IRQ pin 
   */
  tft.getTouchRaw(&x,&y);
  /* 
   *  We need to convert the raw data to screen coordinates
   */
  tft.convertRawXY(&x,&y);

  if( (x<=480) && (y<=320) ){
    /* Depending on the screenrotation we need to shift coordinates */
    if(3 == tft.getRotation() ){
      data->point.x = 480-x;
      data->point.y = 320-y;
    } else if ( 1 == tft.getRotation() ){
      data->point.x = x;
      data->point.y = y;
    } else {
      /* The current config will not support potrait mode */
      assert(NULL);
    }
    last_x=x;
    last_y=y;
    data->state = LV_INDEV_STATE_PR;  

  } else {
    data->state = LV_INDEV_STATE_REL; 
    data->point.x = last_x;
    data->point.y = last_y; 
   
  }
  return moredata;      
}

/**************************************************************************************************
 *    Function      : hal_init
 *    Description   : Sets up the dirver and hooks for littlevgl
 *    Input         : none 
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void hal_init(void)
{
    /*Initialize the display*/
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv); 
    disp_drv.disp_flush = disp_flush_data;
    lv_disp_drv_register(&disp_drv);
    
    /*Initialize the touch screen*/
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv); 
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read = x2064_tp_read;
    lv_indev_drv_register(&indev_drv);

}

/**************************************************************************************************
 *    Function      : lv_tick_handler
 *    Description   : requiered to be called periodically by a ticker 
 *    Input         : none 
 *    Output        : none
 *    Remarks       : none
 **************************************************************************************************/
void lv_tick_handler() {
  lv_tick_inc(LVGL_TICK_PERIOD);
}



/**************************************************************************************************
 *    Function      : setup
 *    Description   : Startupfunction for the Arduinoframworc
 *    Input         : none 
 *    Output        : none
 *    Remarks       : Only called once
 **************************************************************************************************/
void setup() {

   tft.init();
   /* If we set Rotation to 3 we also need to invert X and Y */
   tft.setRotation(3);
   /* Init fir the lvgl library */
   lv_init();
   /* init for the screen and touch driver objects */
   hal_init();
   /* Setup the ticker to have the lvgl get it's timing */
   lvgl_tick.attach_ms(LVGL_TICK_PERIOD, lv_tick_handler);
 
}


/**************************************************************************************************
 *    Function      : loop
 *    Description   : Superloop, called as fast as possible
 *    Input         : none 
 *    Output        : none
 *    Remarks       : This is done in a low priority task
 **************************************************************************************************/
void loop() {
  /* handler to keep the UI going */
  lv_task_handler();
}
