#!/bin/bash
POSE_DIR="/media/lab/E_Disk/Lab_Project/VO_modules/svo_edgelet_ws"
cd $POSE_DIR && source devel/setup.bash
catkin_make
rosrun image_node publisher & sleep 3 && \
rosrun svo_edgelete_live svo_pose

