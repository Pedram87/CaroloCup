/* niftest.c */

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/highgui.h>
#include <opencv/cv.h>

#include "erl_nif.h"

#include <iostream>
#include <sstream>
#include <sys/time.h>
#include <stdio.h>
#include <math.h>

using namespace cv;
using namespace std;


#define LINECOVERAGE 250.0

#define CENTER_X 376
#define CENTER_Y 240

#define LEFT_LINE 3
#define DASHED_LINE 2
#define RIGHT_LINE 1

#define LEFT_MIN 327.0
#define LEFT_MAX 359.0
#define DASHED_MIN 250.0
#define DASHED_MAX 320.0
#define RIGHT_MIN 210.0
#define RIGHT_MAX 238.0

#define PI 3.141592653589793238462


float dist(CvPoint p1, CvPoint p2) {
  return sqrt((p2.y - p1.y) * (p2.y - p1.y) + (p2.x - p1.x) * (p2.x - p1.x));
}

float dist(Point2f p1, Point2f p2) {
  return sqrt((p2.y - p1.y) * (p2.y - p1.y) + (p2.x - p1.x) * (p2.x - p1.x));
}

bool is_trash(vector<vector<Point2i> > line){

  if(line.size() < 5)
    return true;

  for (unsigned int i = 0; i < line.size(); ++i) {
    float length = (((line[i][1].y  - LINECOVERAGE) / (480.0 - LINECOVERAGE)) * 30.0) + 10;
    unsigned int width = line[i][1].x - line[i][0].x;
    if(width > length ){
      //cout << "WIDTH : " << width << " LENGTH "<< length<<  " HOW MUCH LEFT? "<< line.size() - i << "  I  "<<  i <<endl;
      if(line.size() > i + 5 ){
	//cout << "REMOVED" << endl;
	return true;
      }
    }
    if(width > line.size())
      return true;
  }

  return false;
}

float get_angle( Point2f a, Point2f b, Point2f c ){
  Point2f ab = Point2f(b.x - a.x, b.y - a.y );
  Point2f cb = Point2f(b.x - c.x, b.y - c.y );

  float dot = (ab.x * cb.x + ab.y * cb.y); // dot product
  float cross = (ab.x * cb.y - ab.y * cb.x); // cross product

  float alpha = atan2(cross, dot);

  return alpha * 180 / PI;
}


Point2f center_point(Point2f a, Point2f b){
  return Point2f((a.x+b.x)/2 , (a.y+b.y)/2 );
}

int find_dashed(vector<vector<vector<vector<Point2i> > > > grouped){
  for (unsigned int i = 0; i < grouped.size(); i++) {
    if(grouped[i].size() > 1){
      bool correct = true;
      for (unsigned int j = 0; j < grouped[i].size() - 1; j++) {
	int last = grouped[i][j].size() -1;
	Point2f start = center_point(grouped[i][j][last][0] , grouped[i][j][last][1]);

	Point2f end  = center_point(grouped[i][j+1][0][0] , grouped[i][j+1][0][1]);
	float distance =  start.y - end.y;// dist(start, end);
	float length = (start.y - 250) / 160 * 65 + 8;
	if(distance > length){
	  correct = false;
	}
      }
      if(correct){
	return i;
      }
    }
  }
  return -1;
}



ErlNifResourceType* frame_res = NULL;

typedef struct _range {
  int left;
  int mid;
  int right;
  int length;
} range;

typedef struct _frame_t {
  IplImage* _frame;
} frame_t;

//------------------------------------------------------------------------------
// NIF callbacks
//------------------------------------------------------------------------------

static void frame_cleanup(ErlNifEnv* env, void* arg) {
  enif_free(arg);
}

static int load(ErlNifEnv* env, void** priv, ERL_NIF_TERM load_info)
{

  ErlNifResourceFlags flags = (ErlNifResourceFlags) (ERL_NIF_RT_CREATE | ERL_NIF_RT_TAKEOVER);
  frame_res = enif_open_resource_type(env, "niftest", "ocv_frame",
				      &frame_cleanup,
				      flags, 0);
  return 0;
}


static ERL_NIF_TERM get_pic(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{

  IplImage* src = cvLoadImage("/home/khashayar/Downloads/pic.png");

  //  cout << src->width << endl;

  IplImage* gray = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
  cvCvtColor(src, gray, CV_RGB2GRAY);


  frame_t* frame = (frame_t*)enif_alloc_resource(frame_res, sizeof(frame_t));

  frame->_frame = gray ;

  ERL_NIF_TERM term = enif_make_resource_binary(env, frame, frame ,6);

  //enif_release_resource(frame);

  return enif_make_tuple2(env, enif_make_atom(env, "ok"), term); 
  
}




static ERL_NIF_TERM show_pic(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]){

  frame_t* frame;
  if (!enif_get_resource(env, argv[0], frame_res, (void**) &frame)) {
    return enif_make_badarg(env);
 } 

  cvShowImage("YOOHOO", frame->_frame);

  cvWaitKey(30);
  
  return enif_make_atom(env, "ok");
}


static ERL_NIF_TERM read_complete(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]){

  frame_t* frame;
  if (!enif_get_resource(env, argv[0], frame_res, (void**) &frame)) {
    return enif_make_badarg(env);
  }

  //cout << "PIC : " << frame->_frame->width << endl;

  //cout << "SEGMENT : " << segment << endl;

  ERL_NIF_TERM res[480*752];

  int count = 0; 
  int i;
  for( i = 0; i < 480*752; i++)
    {

      if(frame->_frame->imageData[i] > 100){
	int y = i / 752;
	int x = i % 752;
	ERL_NIF_TERM erl_x = enif_make_int(env, x);
	ERL_NIF_TERM erl_y = enif_make_int(env, y);
	res[count] = enif_make_tuple2(env, erl_x, erl_y);
	count++;
      }
    }

  //cout << endl << "DONE WITH FOR : " << count << endl;

  // enif_release_resource(frame);

  ERL_NIF_TERM points_erl = enif_make_list_from_array(env, res, count);
  //free(res);

  //cout << "ARRAY HERE READY" << endl;
  return points_erl;
}


static ERL_NIF_TERM read_part(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]){

  frame_t* frame;
  if (!enif_get_resource(env, argv[0], frame_res, (void**) &frame)) {
    return enif_make_badarg(env);
  }

  //cout << "PIC : " << frame->_frame->width << endl;

  int segment;
  enif_get_int(env, argv[1], &segment);
  
  //cout << "SEGMENT : " << segment << endl;

  ERL_NIF_TERM res[480*752/4];

  int count = 0; 
  int i;

  for( i = (segment -1) * (480*752/4); i < segment * 480*752/4; i++)
    {

      if(frame->_frame->imageData[i] > 100){
	int y = i / 752;
	int x = i % 752;
	ERL_NIF_TERM erl_x = enif_make_int(env, x);
	ERL_NIF_TERM erl_y = enif_make_int(env, y);
	res[count] = enif_make_tuple2(env, erl_x, erl_y);
	count++;
      }
    }
  //cout << endl << "DONE WITH FOR : " << count << endl;

  // enif_release_resource(frame);

  ERL_NIF_TERM points_erl = enif_make_list_from_array(env, res, count);
  //free(res);

  //cout << "ARRAY HERE READY" << endl;
  return points_erl;
}

static ERL_NIF_TERM process_complete(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]){

  frame_t* frame;
  if (!enif_get_resource(env, argv[0], frame_res, (void**) &frame)) {
    return enif_make_badarg(env);
  }

  //cout << "PIC : " << frame->_frame->width << endl;


  ERL_NIF_TERM res[480*752];

  int count = 0; 
  int i,j;

  int min_x=751, max_x=0;
  
  int plus_minus = 10;

  int row = 479*752;
  bool found = false;
  while(!found)
    {
      for( i = 0; i< 752; i++)
	{
	  if((uchar)(frame->_frame->imageData[row + i]) > 100)
	    {
	      int y = row / 752;
	      int x = i ;
	      ERL_NIF_TERM erl_x = enif_make_int(env, x);
	      ERL_NIF_TERM erl_y = enif_make_int(env, y);
	      cvLine(frame->_frame,cvPoint(x,y),cvPoint(x,y),CV_RGB(0,0,0),1,8,0);	      
	      res[count] = enif_make_tuple2(env, erl_x, erl_y);
	      count++;
	      if(min_x > x)
		min_x = x;
	      if(max_x < x)
		max_x = x;
	      found = true;
	      break;
	    }
	}
      row -= 752;
    }
  
  for(i=row; i>250*752; i=i-752)
    {
      for(j= MAX(0,min_x - plus_minus); j< MIN(752, min_x + (2*plus_minus) ); j++)
	{
	  if((uchar) (frame->_frame->imageData[i+j]) > 100)
	    {
	      int y = i / 752;
	      int x = j ;
	      ERL_NIF_TERM erl_x = enif_make_int(env, x);
	      ERL_NIF_TERM erl_y = enif_make_int(env, y);
	      cvLine(frame->_frame,cvPoint(x,y),cvPoint(x,y),CV_RGB(0,0,0),1,8,0);	      
	      res[count] = enif_make_tuple2(env, erl_x, erl_y);
	      count++;
	      if(min_x > x)
		min_x = x;
	      if(max_x < x)
		max_x = x;
	    }
	} 
    }

  

 
  //cout << endl << "DONE WITH FOR : " << count << endl;

  // enif_release_resource(frame);

  ERL_NIF_TERM points_erl = enif_make_list_from_array(env, res, count);
  //free(res);
    cvShowImage("Drawing_and_Text", frame->_frame);
  cvWaitKey();

  //cout << "ARRAY HERE READY" << endl;
  return points_erl;
}


static ERL_NIF_TERM trace_pic(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]){

  frame_t* frame;
  if (!enif_get_resource(env, argv[0], frame_res, (void**) &frame)) {
    return enif_make_badarg(env);
  }
  int current_most_left = 751;
  int current_most_right = 0;
  int current_most_right_temp = 0;

  vector<vector<Point2i> > lanes;
  vector<Point2i> current_line;
  int plus_minus = 10;
  int row = 479, column;

  int current_row;
  int current_column;

  //cvLine(src,cvPoint(0,479),cvPoint(751,479), CV_RGB(0,0,255));
  bool bottom_found;
  while (row > 250/*most_covered_right - most_covered_left < 500*/) {
    //color++;
    bottom_found = false;
    column = 0;

    while (!bottom_found) {
      //			cout << "HERE1" << endl;
      for (column = 0; column < 751; column++) {
	if ((uint) (frame->_frame->imageData[row * 752 + column]) > 100) {
	  current_line.push_back(Point2i(column, row));
	  //					gray->imageData[current_row * 752 + current_column] = (char) 0;
	  /*
	  cvLine(gray, cvPoint(current_column, current_row),
	  	 cvPoint(current_column, current_row),
		 CV_RGB(0, 0, 0));
	  cvLine(src, cvPoint(column, row), cvPoint(column, row),
		 colors[color%3]);
	  */
	  current_row = row;
	  current_most_left = column;
	  current_most_right = column;
	  current_most_right_temp = column;
	  bottom_found = true;
	  break;
	}
      }
      row--;
    }

    //		cout << column << "," << row << endl;
    bool valid_row = true;
    while (valid_row && current_row > 250) {
      //			cout << "HERE2" << endl;
      valid_row = false;
      current_column = current_most_left - plus_minus;
      current_most_right = current_most_right_temp;
      current_most_right_temp = 0;
      int on_white = 0;
      bool hit_white = false;
      while ((on_white > 0 || !hit_white)
	     && current_column < current_most_right + plus_minus) {
	if ((uint) (frame->_frame->imageData[current_row * 752 + current_column])
	    > 100) {
	  //					cout << current_column << ", " << current_row << endl;
	  //					gray->imageData[current_row * 752 + current_column] = (char)0;
	  current_line.push_back(Point2i(column, row));
	  /*
	  cvLine(src, cvPoint(current_column, current_row),
		 cvPoint(current_column, current_row),
		 colors[color%3]);
	  cvLine(gray, cvPoint(current_column, current_row),
		 cvPoint(current_column, current_row),
		 CV_RGB(0, 0, 0));
	  */
	  if(current_column< current_most_left)
	    current_most_left = current_column;
	  if(current_column> current_most_right_temp)
	    current_most_right_temp = current_column;
	  hit_white = true;
	  on_white = 3;
	  valid_row = true;
	} else {
	  on_white--;
	}
	current_column++;
      }
      current_row--;
    }
  }
  return enif_make_int(env, current_line.size());
}
static ERL_NIF_TERM process_pic(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]){
  /* GET IMG FROM CAMERA
  frame_t* frame;
  if (!enif_get_resource(env, argv[0], frame_res, (void**) &frame)) {
    return enif_make_badarg(env);
  } 
  IplImage* gray = frame->_frame;
     GET IMG FROM CAMERA */

  IplImage* src = cvLoadImage("/home/robin/Downloads/pic.png");
  IplImage* gray = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 1);
  cvCvtColor(src, gray, CV_RGB2GRAY);

  int row = 423, column;

  int current_row;

  range line_range;
  line_range.length = 25;

  vector<vector<vector<Point2i> > > lanes;

   
  while (row > LINECOVERAGE) {
    bool bottom_found;
    bottom_found = false;
    vector<vector<Point2i> > current_line;
    while (!bottom_found) {
      for (column = 151; column < 600; column++) {
	if ((uint) (gray->imageData[row * 752 + column]) > 100) {
	  gray->imageData[row * 752 + column] = (char) 0;
	  current_row = row;
	  bottom_found = true;
	  line_range.mid = column;
	  break;
	}
      }
      row -= 5;
    }
      
    int valid_row = 3;
    while (valid_row > 0) {
      valid_row = false;
      int on_white = 0;
      bool hit_white = false;

      line_range.length = (((current_row - LINECOVERAGE) / (480.0 - LINECOVERAGE)) * 30.0) + 10;

      line_range.left = MAX(151,line_range.mid - line_range.length);
      line_range.right = MIN(600,line_range.mid + line_range.length);

      column = line_range.left;

      vector<Point2i > single_line;

      while ( (!hit_white && column < line_range.right) || (on_white > 0 && column < 600)) {
	if ((uint) (gray->imageData[current_row * 752 + column]) > 100) {
	  gray->imageData[current_row * 752 + column] = (char) 0;
	  single_line.push_back(Point2i(column, current_row));
	  hit_white = true;
	  on_white = 3;
	  valid_row = 3;
	} else {
	  on_white--;
	}
	column++;
      }
      if (valid_row ==  3) {

	CvPoint center;
	center.x = (single_line[0].x + single_line[single_line.size() - 1].x) / 2;
	center.y = current_row;

	vector<Point2i> left_right_points;
	left_right_points.push_back(Point2i(single_line[0].x ,current_row));
	left_right_points.push_back(Point2i(single_line[single_line.size() - 1].x ,current_row));

	current_line.push_back(left_right_points);
	line_range.mid = center.x;
      } else {
	valid_row--;
      }
      current_row--;
    }
    if (current_line.size() > 1 )
      lanes.push_back(current_line);

  } // end of frame while


  unsigned int lanes_size = lanes.size();
  for (unsigned int i = 0; i < lanes.size(); i++) {
    if(lanes[i].size() > 11){
      for (unsigned int j = 5; j < lanes[i].size()-6; j++) {
	Point2f start;
	start.x = (lanes[i][j-5][0].x + lanes[i][j-5][1].x) / 2;
	start.y = lanes[i][j-5][0].y;

	Point2f end;
	end.x = (lanes[i][j+5][0].x + lanes[i][j+5][1].x) / 2;
	end.y = lanes[i][j+5][0].y;

	Point2f center;
	center.x = (lanes[i][j][0].x + lanes[i][j][1].x) / 2;
	center.y = lanes[i][j][0].y;

	float alpha = get_angle(end,center,start);
	if((alpha > 80) || ( alpha < -40) ){
	}else{
	  vector<vector<Point2i> > temp;
	  int c= 0;
	  for (unsigned int k = j; k < lanes[i].size(); k++) {
	    temp.push_back(lanes[i][k]);
	    c++;
	  }
	  lanes.push_back(temp);
	  for (int k = 0; k < c; k++) {
	    lanes[i].pop_back();
	  }
	  j = lanes[i].size();
	  break;
	}
      }
    }
  }

  vector<vector<vector<vector<Point2i> > > > grouped;

  vector<vector<vector<Point2i> > > temp;
  temp.push_back(lanes[0]);
  grouped.push_back(temp);

  for (unsigned int i = 1; i < lanes_size; i++) {
    bool connected = false;
    Point2f lanes_center = Point2f((lanes[i][0][0].x  + lanes[i][0][1].x)/2 , lanes[i][0][0].y);
    for (unsigned int j = 0; j < grouped.size(); j++) {
      Point2f grouped_point_left = grouped[j][grouped[j].size()-1][grouped[j][grouped[j].size()-1].size()-1][0];
      Point2f grouped_point_right = grouped[j][grouped[j].size()-1][grouped[j][grouped[j].size()-1].size()-1][1];
      Point2f grouped_point_center = Point2f((grouped_point_left.x + grouped_point_right.x)/2 , grouped_point_left.y);
      float distance_group = dist(grouped_point_center , lanes_center);

      if(grouped_point_center.y >= lanes_center.y && distance_group < 100){

	int first  = div(grouped[j][grouped[j].size()-1].size() , 4).quot;
	int last = grouped[j][grouped[j].size()-1].size() - first -1;
	Point2f start = center_point(grouped[j][grouped[j].size()-1][first][0], grouped[j][grouped[j].size()-1][first][1]);
	Point2f end = center_point(grouped[j][grouped[j].size()-1][last][0], grouped[j][grouped[j].size()-1][last][1]);
	int x = (((start.x - end.x) * (lanes_center.y - start.y)) / (start.y - end.y)) + start.x;

	float distance = dist(lanes_center , Point2f(x,lanes_center.y));
	if(distance < 25){
	  if(!is_trash(lanes[i])){
	    grouped[j].push_back(lanes[i]);
	  }
	  connected = true;
	}
      }//end of if(grouped_point_center... )
	

    }//end of for(unsigned int j... )
    if(!connected){
      if(!is_trash(lanes[i])){
	temp.clear();
	temp.push_back(lanes[i]);
	grouped.push_back(temp);
      }
    }
  }//end of for(unsigned int i... )


  int dash_index = find_dashed(grouped);

  vector<Point2f> final_result;
  for(unsigned int i = 0; i< grouped[dash_index].size() ; i++)
    {
      Point2f sum = Point2f(0,0); 
      int count = 0;
      for(unsigned int j = 0; j< grouped[dash_index][i].size() ; j++)
	{
	  Point2f grouped_point_left = grouped[dash_index][i][j][0];
	  Point2f grouped_point_right = grouped[dash_index][i][j][1];
	  sum.x += grouped_point_left.x;
	  sum.x += grouped_point_right.x;
	  sum.y += grouped_point_left.y;
	  sum.y += grouped_point_right.y;
	  count ++;
	}
      Point2f avg = Point2f(sum.x / count , sum.y / count);
      final_result.push_back(avg);
    }
  
  ERL_NIF_TERM result_erl[final_result.size()];

  int i;
  for( i = 0; i < final_result.size(); i++)
    {
      int y = final_result[i].y;
      int x = final_result[i].x;
      ERL_NIF_TERM erl_x = enif_make_int(env, x);
      ERL_NIF_TERM erl_y = enif_make_int(env, y);
      result_erl[i] = enif_make_tuple2(env, erl_x, erl_y);
    }

  ERL_NIF_TERM points_erl = enif_make_list_from_array(env, result_erl, final_result.size());

  return points_erl;
}

static ErlNifFunc nif_funcs[] =
  {
    {"show_pic", 1, show_pic},
    {"process_pic", 0, process_pic},
    {"trace_pic", 1, trace_pic},
    {"get_pic", 0, get_pic},
    {"read_part", 2, read_part},
    {"read_complete", 1, read_complete},
    {"process_complete", 1, process_complete}
  };

ERL_NIF_INIT(niftest,nif_funcs,load,NULL,NULL,NULL)
