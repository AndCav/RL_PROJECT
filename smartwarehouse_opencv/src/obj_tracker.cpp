#include "obj_tracker.h"
using namespace std;
using namespace cv;
using namespace tf;

bool first_col=true;

int low_r = 5;
int low_g = 0;
int low_b = 0;

int high_r = 225;
int high_g = 1;
int high_b = 1;


void on_low_r_thresh_trackbar(int, void *) {
    low_r = min(high_r-1, low_r);
    setTrackbarPos("Low R","RGB", low_r);
}
void on_high_r_thresh_trackbar(int, void *) {
    high_r = max(high_r, low_r+1);
    setTrackbarPos("High R", "RGB", high_r);
}
void on_low_g_thresh_trackbar(int, void *) {
    low_g = min(high_g-1, low_g);
    setTrackbarPos("Low G","RGB", low_g);
}
void on_high_g_thresh_trackbar(int, void *) {
    high_g = max(high_g, low_g+1);
    setTrackbarPos("High G", "RGB", high_g);
}
void on_low_b_thresh_trackbar(int, void *) {
    low_b = min(high_b-1, low_b);
    setTrackbarPos("Low B","RGB", low_b);
}
void on_high_b_thresh_trackbar(int, void *) {
    high_g = max(high_g, low_g+1);
    setTrackbarPos("High B", "RGB", high_b);
}

//LOAD TOPIC
void load_param( string & p, string def, string name ) {
  ros::NodeHandle n_param("~");
  if( !n_param.getParam( name, p))
    p = def;
  cout << name << ": " << "\t" << p << endl;
}

void load_param( int & p, int def, string name ) {
  ros::NodeHandle n_param("~");
  if( !n_param.getParam( name, p))
    p = def;
  cout << name << ": " << "\t" << p << endl;
}

//SET_RGB
void load_param( bool & p, bool def, string name ) {
  ros::NodeHandle n_param("~");
  if( !n_param.getParam( name, p))
    p = def;
  cout << name << ": " << "\t" << p << endl;
}



box_tracking::box_tracking() {
    
    string _img_topic;
    string _depth_img_topic;
    string _cam_info_topic;

    load_param( _img_topic, "/depth_camera/depth_camera/image_raw", "img_topic" );
    load_param( _depth_img_topic, "/depth_camera/depth/disparity", "depth_img_topic"); 
    load_param( _cam_info_topic, "/depth_camera/depth_camera/camera_info", "camera_info_topic" );
    load_param( _rate, 30, "rate" );
    load_param( _set_RGB, false, "set_RGB");
    
    load_param( _low_r_1, 5, "low_r_1");
    load_param( _low_g_1, 0, "low_g_1");
    load_param( _low_b_1, 0, "low_b_1");
    load_param( _high_r_1, 225, "high_r_1");
    load_param( _high_g_1, 1, "high_g_1");
    load_param( _high_b_1, 1, "high_b_1");
    
    load_param( _low_r_2, 0, "low_r_2");
    load_param( _low_g_2, 0, "low_g_2");
    load_param( _low_b_2, 5, "low_b_2");
    load_param( _high_r_2, 1, "high_r_2");
    load_param( _high_g_2, 1, "high_g_2");
    load_param( _high_b_2, 225, "high_b_2");
    
    _img_sub = _nh.subscribe( _img_topic.c_str(), 0, &box_tracking::cam_cb, this );
    _depth_img_sub = _nh.subscribe( _depth_img_topic.c_str(),0, &box_tracking::depth_cb, this );
    _cam_info_sub = _nh.subscribe(_cam_info_topic.c_str(), 0, &box_tracking::cam_parameters, this);
  
    _img_ready = _depth_ready = _cam_info_first = false;
  
}


void box_tracking::cam_cb( sensor_msgs::ImageConstPtr img ) {

	cv_bridge::CvImagePtr cv_ptr;
	try {
		cv_ptr = cv_bridge::toCvCopy(img, sensor_msgs::image_encodings::BGR8); 
        _src = cv_ptr->image;
        _img_ready = true;
	}
	catch (cv_bridge::Exception& e) {
		ROS_ERROR("cv_bridge exception: %s", e.what());
		return;
	}
}

void box_tracking::depth_cb( sensor_msgs::ImageConstPtr depth ) {

    cv_bridge::CvImagePtr cv_ptr;
    try {
        cv_ptr = cv_bridge::toCvCopy(depth, sensor_msgs::image_encodings::TYPE_32FC1); 
        _depth_src = cv_ptr->image;
        _depth_ready = true;
    }
    catch (cv_bridge::Exception& e) {
        ROS_ERROR("cv_bridge exception: %s", e.what());
        return;
    }
}

void box_tracking::tune_rgb_gain() {

    Mat img;
    while( !_img_ready ) { 
        usleep(0.1*1e6);
    }
    
    low_r = _low_r_1;
    low_g = _low_g_1;
    low_b = _low_b_1;

    high_r = _high_r_1;
    high_g = _high_g_1;
    high_b = _high_b_1;

    namedWindow("RGB", WINDOW_NORMAL);
    
    createTrackbar("Low R", "RGB", &low_r,  255, on_low_r_thresh_trackbar);
    createTrackbar("High R","RGB", &high_r, 255, on_high_r_thresh_trackbar);
    createTrackbar("Low G", "RGB", &low_g,  255, on_low_g_thresh_trackbar);
    createTrackbar("High G","RGB", &high_g, 255, on_high_g_thresh_trackbar);
    createTrackbar("Low B", "RGB", &low_b,  255, on_low_b_thresh_trackbar);
    createTrackbar("High B","RGB", &high_b, 255, on_high_b_thresh_trackbar);

    ros::Rate r(_rate);

    while(ros::ok()) {
        Mat img = _src;

        if( img.empty() )
            continue;
      
        inRange(img,Scalar(low_b, low_g, low_r), Scalar(high_b, high_g, high_r), img);
        imshow( "RGB", img );
        
        waitKey(1);
      
        img.release();
    }
} 




void box_tracking::track_rectangle() {
    ros::Rate r(_rate);
    Mat img;
    Mat depth;
    cout << "Waiting for server" << endl;
    while( !_img_ready || !_depth_ready || !_cam_info_first) {
        usleep(0.1*1e6);
    }  
  actionlib::SimpleActionClient<smartwarehouse_opencv::box_posAction> a_c("abb_robot_server", true);
  //a_c.waitForServer(); //will wait for infinite time
  cout << "Server is ready" << endl;
  smartwarehouse_opencv::box_posGoal pos_goal;
    int low_rgb[3]; int high_rgb[3];  
    vector<Point> rectangle_center;
    double cx, cy, fx_inv, fy_inv;
    double cx_c1, cy_c1, cz_c1;
    float zd_c1;
    float zd_c2;
    Matrix3x3 Rot_matrix_;
    
    while(ros::ok()) {
      bool box_moved = false;
      img = _src;
      depth = _depth_src;
       if(first_col){ //RED
          low_rgb[0] = _low_r_1;
          low_rgb[1] = _low_g_1;
          low_rgb[2] = _low_b_1;
          high_rgb[0] = _high_r_1;
          high_rgb[1] = _high_g_1;
          high_rgb[2] = _high_b_1;
       }
       else{           //BLUE
          low_rgb[0] = _low_r_2;
          low_rgb[1] = _low_g_2;
          low_rgb[2] = _low_b_2;
          high_rgb[0] = _high_r_2;
          high_rgb[1] = _high_g_2;
          high_rgb[2] = _high_b_2;
       }
      
      
      if(get_rectangle( img, low_rgb, high_rgb,  rectangle_center)){    
        cx = _cam_cameraMatrix->at<double>(0,2);
        cy = _cam_cameraMatrix->at<double>(1,2);
        fx_inv = 1.0 / _cam_cameraMatrix->at<double>(0,0);
        fy_inv = 1.0 / _cam_cameraMatrix->at<double>(1,1);
        
        for(int i=0; i< rectangle_center.size(); i++){
           zd_c1 = depth.at<float>(rectangle_center[i].y,rectangle_center[i].x);
           cx_c1 = (zd_c1) * ( (rectangle_center[i].x - cx) * fx_inv );
           cy_c1 = (zd_c1) * ( (rectangle_center[i].y - cy) * fy_inv );
           cz_c1 = zd_c1;
         // cout << "Position in camera frame: (" << cx_c1 << ", " << cy_c1 << ", " << cz_c1 << ")" << endl; 
          
          Rot_matrix_.setRPY(3.14, 0, -1.57);
          double px=(Rot_matrix_[0].x() * cx_c1)+(Rot_matrix_[0].y() * cy_c1)+(Rot_matrix_[0].z()* cz_c1) - 1.5;
          double py=(Rot_matrix_[1].x() * cx_c1)+(Rot_matrix_[1].y() * cy_c1)+(Rot_matrix_[1].z()* cz_c1) -0.02;
          double pz=(Rot_matrix_[2].x() * cx_c1)+(Rot_matrix_[2].y() * cy_c1)+(Rot_matrix_[2].z()* cz_c1) + 1.88;  
         
          pos_goal.x_box = px;
          pos_goal.y_box = py;
          pos_goal.z_box = pz;
           cout<<"Send pos in world frame n."<<i<<": "<<px<<" "<<py<<" "<<pz<<endl;
          a_c.sendGoal(pos_goal);
          while(!box_moved){
            usleep(1);
            
            if (a_c.getState() == actionlib::SimpleClientGoalState::SUCCEEDED ) {
		                cout << "Box successfully moved!" << endl;
			            box_moved = true;
			           r.sleep();
		       }
		       else if(a_c.getState() == actionlib::SimpleClientGoalState::ABORTED) {cout<<"The goal was ABORTED by the action server due to some failure"<<endl;}
          }
          
          
          /*Rot_matrix_.setRPY(0, 0, 3.14);
          double pxr=(Rot_matrix_[0].x() * px)+(Rot_matrix_[0].y() * py)+(Rot_matrix_[0].z()* pz) +0.5;
          double pyr=(Rot_matrix_[1].x() * px)+(Rot_matrix_[1].y() * py)+(Rot_matrix_[1].z()* pz) -1.5;
          double pzr=(Rot_matrix_[2].x() * px)+(Rot_matrix_[2].y() * py)+(Rot_matrix_[2].z()* pz) ;//-0.1  
          cout<<"POS in robot base frame n."<<i<<": "<<pxr<<" "<<pyr<<" "<<pzr<<endl;*/
          
         }
       }
       else{
         cout<<"FINISHED FIRST (default RED)"<<endl;
         first_col=false;
       }
      
        r.sleep();
    }
}


void box_tracking::run() {

  if( _set_RGB )
    boost::thread tune_rgb_gain_t( &box_tracking::tune_rgb_gain, this );
  else 
    boost::thread track_rectangle_t( &box_tracking::track_rectangle, this );
 
  ros::spin();
}




int main(int argc, char** argv) {
    ros::init( argc, argv, "object_tracker");
    box_tracking st;
    st.run();

    return 0;

}



