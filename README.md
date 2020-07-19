# Smart Warehouse 2 - Robotics Lab 2020 - Master's Degree Automation Engineering University of Naples Federico II  

A ROS/Gazebo Warehouse_2 project for Robotics Lab made by Eliana La Frazia and Andrea Cavaliere (andreacavaliere49@gmail.com and lafrazia.eliana@gmail.com). The pioneer folder is an adaption of the Pioneer model made by Rafael Berkvens and Mario Serna Hernández. 
More details in the report.

# UPDATES
```
update introduces new features. 
-It is now available to launch the simulation just with two .launch file:
  1) launch the world and  pioneers' nodes
  $ roslaunch smart_warehouse_2 all.launch
  using the parameter twopioneer(boolean parameter) you can choose to simulate the warehouse with 1 or 2 pioneers( obviously this will influence the box management policy)
  2) launch the camera and manipulator node
  $ roslaunch smart_warehouse_2 manipulator.launch

-The camera node now has a better and smarter policy to choose the box to pick (see the report for details).

-The manipulator movements are now smoother and faster.

-The robots adapt to every movement of the object in the world.

-any position change can be done in the .launch file

-

-The RViz window doesn't open anymore, if the user wants to use it, or at least see it, he only needs to uncomment from row 37 to 39 in  file: /RL_PROJECT/abb_irb6640_gazebo/launch/moveit_planning_execution_gazebo.launch

-to make the "ray beam" appear during the simulation go to pioneer_p3dx_model/p3dx_description/urdf/pioneer3dx.gazebo, row 97 and set "visualize" to true.
```

# To install:
```
$ cd <catkin_ws>/src
$ git clone https://github.com/elianalf/RL_PROJECT.git 
$ cd ..
$ catkin_make

If there are same actionlib errors during the compilation of the packages "smart_warehouse_2" and "smartwarehouse_opencv" it is recommended:
$ catkin_make -DCATKIN_WHITELIST_PACKAGES="smart_warehouse_2"
$ catkin_make -DCATKIN_WHITELIST_PACKAGES="smartwarehouse_opencv"
```

# To use:
```
To make sure your workspace is properly overlayed by the setup script:
$ source <catkin_ws>/devel/setup.sh
to launch the system see Execution Example

```
# Necessary dependencies :

```
--move base and navigation-- 
move_base 
$  sudo apt-get install ros-melodic-move-base ros-melodic-move-base-msgs 
gmapping 
$  sudo apt-get install ros-melodic-gmapping
amcl
$ sudo apt-get install ros-melodic-amcl

$ sudo apt-get install ros-melodic-map-server
$ sudo apt-get install ros-melodic-dwa-local-planner

--OpenCV--
OpenCv can be compiled and installed after downloaded the desired version on this page: https://github.com/opencv/opencv/tree/4.3.0

--Move-it!, ros-industrial and ABB stack--
$ sudo apt-get install ros-melodic-moveit ros-melodic-moveit-plugins ros-melodic-moveit-planners
$ sudo apt-get install ros-melodic-industrial-core
$ sudo apt-get install ros-melodic-abb

You also need to install Eigen:
$ sudo apt-get install libeigen3-dev

---probably already downloaded dependencies but needed--
gazebo
gazebo_ros
gazebo_ros_control
gazebo_ros_pkgs
joint_state_controller
position_controllers
joint_trajectory_controller
robot_state_publisher
ros_controllers
```

# Execution Example:
```
If the user needs to tune the rgb exact values the camera has to detect, he/she can follow this step.  The default colours are red and blue.
Every step has to be done in all separate shells:

1)Launching Gazebo and spawning robots
$ roslaunch smart_warehouse_2 warehouse2.launch

2)Go to /smartwarehouse_opencv/launch/identify_boxes.launch and change the value of the parameter "set_RGB" to TRUE and launch it.
$ roslaunch smartwarehouse_opencv identify_boxes.launch

3)Define the desired ranges of rgb values and change them into the identify_boxes.launch. Then change the value of the parameter "set_RGB" to FALSE and launch it again as the previous step 2). Finally execute the step 2) (Launch Move-it! Core) and from step 4) onwards of the following steps. 
```
```
If the user doesn't need to tune the rgb values, he/she can follow this step.

Every step has to be done in all separate shells:

_OLD PROCEDURE:_
 
1)Launching Gazebo and spawning robots
$ roslaunch smart_warehouse_2 warehouse2.launch

2)Launch Move-it! Core. After the launch it's possible to close the gui of RVIZ (RVIZ window).
$ roslaunch abb_irb6640_gazebo moveit_planning_execution_gazebo.launch 

3)Run object-tracker node 
$ rosrun smartwarehouse_opencv obj_tracker

4)Launch pioneer move base and AMCL
$ roslaunch p3dx_move moving_map.launch

5)Run Pioneer robot node
$ rosrun p3dx_move pioneer_p3dx_1
EDIT: the correct command now would be "$ rosrun p3dx_move pioneer_p3dx p3dx_1" or in general "$rosrun p3dx_move pioneer_p3dx {name_of_robot}" being careful to use the same name  spawning the robot, launching the related move_base,AMCL nodes.

6) Run Pioneer manager node
$ rosrun p3dx_move p3dx_manager

7)Run ABB pick and place task manager
$ rosrun smart_warehouse_2 abb_pp_task 

_NEW PROCEDURE:_

1) open gazebo, spawn all the robots and start the AGV nodes
$roslaunch smart_warehouse_2 all.launch


2)start camera and manipulator
$roslaunch smart_warehouse_2 manipulator.launch


as an alternative to this procedure you can launch 3 different files:

1) one to start the gazebo world and spawn all the robots
$ roslaunch smart_warehouse_2 warehouse2.launch

2) the second one to start all the AGVs nodes
$ roslaunch smart_warehouse_2 pioneer.launch

2) the last one to start camera and manipulator
$ roslaunch smart_warehouse_2 manipulator.launch

```


# Note:
```
It is possible to launch  one or two pioneers just setting to false the variable "Twopioneer".

```
