#include <ros/ros.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include <geometry_msgs/Pose.h>
#include <std_msgs/Bool.h>
#include <std_msgs/Int8.h>
typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;


bool newdata;
move_base_msgs::MoveBaseGoal _goal;
move_base_msgs::MoveBaseGoal _base;
std::string pref;

//LOAD COORDINATE COMPONENT
void load_param( double & p, double def, std::string name ) {
  ros::NodeHandle n_param;
  if( !n_param.getParam( name, p))
    p = def;
  std::cout << name << ": " << "\t" << p << std::endl;
}
	
void callback(geometry_msgs::Pose destination){
_goal.target_pose.header.frame_id = "map";
ROS_INFO("new message");

_goal.target_pose.pose.position.x =destination.position.x;
_goal.target_pose.pose.position.y = destination.position.y;
_goal.target_pose.pose.orientation.w = destination.orientation.w;
newdata=true;
}
	


int main(int argc, char** argv){
	ROS_INFO("starting node");

	if(argc<2){ pref="p3dx_1";}
	else{ pref=argv[1];}
   	ros::init(argc, argv, "pioneer_p3dx");
	ros::NodeHandle _nh;
	ros::Subscriber _topic_sub;
	ros::Publisher _topic_pub;
	
	std::string buff=pref+"/move_base";
	MoveBaseClient _ac(buff, true);
	
	ROS_INFO("parameters inizialization done");
	while(!_ac.waitForServer()){
    ROS_INFO("Waiting for the move_base action server to come up");
  }	
  	buff="/Pion/"+pref+"/destination";
  	_topic_sub = _nh.subscribe(buff, 3,callback);
  	buff="/Pion/"+pref+"/PosReached";
	_topic_pub =_nh.advertise<std_msgs::Int8>(buff,3);
	ROS_INFO("topic started");
	
	
	double buff_coordinate=0.0;
	
	buff=pref+"_x";
	load_param(buff_coordinate, 0.0, buff );
	_base.target_pose.pose.position.x=buff_coordinate;
	
	buff=pref+"_y";
	load_param(buff_coordinate, 0.0, buff );
	_base.target_pose.pose.position.y=buff_coordinate;
	
	_base.target_pose.pose.orientation.w = 6.0;
	_base.target_pose.header.frame_id = "map";
	
	
	newdata=false;
	ROS_INFO("newdata :%d",newdata);
	ros::Rate r(5);
	std_msgs::Int8 reached;
	std_msgs::Int8 not_reached;
	reached.data=0;
	not_reached.data=3;
	ROS_INFO("starting the loop");
	while(ros::ok()){
		if(newdata){
		newdata=false;
		  ROS_INFO("Sending goal");
		  _goal.target_pose.header.stamp = ros::Time::now();
		  _ac.sendGoal(_goal);

		  _ac.waitForResult();
		  
		  if(_ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED){
		    ROS_INFO("Hooray, the base has almost finished");
		    
		    reached.data=1;
		    _topic_pub.publish(reached);
		    usleep(10000);		    		    
		    _base.target_pose.header.stamp = ros::Time::now();
		    _ac.sendGoal(_base);
		    _ac.waitForResult();
		    if(_ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED){
		    		ROS_INFO("Hooray, the base is back");
				reached.data=2;
				_topic_pub.publish(reached);
		  
		    	}
		    }
		  else{reached.data=3;
				_topic_pub.publish(reached); //retry
				_ac.sendGoal(_base);
		  	}
		    }
	
	ros::spinOnce();
	r.sleep();
	}
  
  return 0;
}
