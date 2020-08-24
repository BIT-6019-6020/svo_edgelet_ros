//
// Created by lab on 20-5-25.
//
#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <iostream>


#include <iomanip>
#include <vector>
#include "/home/lab/cpp_lib/getfile.h"

using namespace cv;
using namespace std;

string config_path =
	"/media/lab/E_Disk/Lab_Project/VO_modules/svo_edgelet_ws/src/image_node/src/input_data.yaml";

string data_pub_topic;
string data_path;
int start_time;
int data_type;
int should_resize;
int input_width;
int input_height;
cv::Size pub_size;
int input_rate;

void readParam(string config_file){
	FILE *fh = fopen(config_file.c_str(),"r");
	if(fh == NULL){
		ROS_WARN("config_file dosen't exist; wrong config_file path");
		ROS_BREAK();
		return;
	}
	fclose(fh);

	cv::FileStorage fsSettings(config_file, cv::FileStorage::READ);
	if(!fsSettings.isOpened())
	{
		std::cerr << "ERROR: Wrong path to settings" << std::endl;
	}
	fsSettings["data_pub_topic"] >> data_pub_topic;
	fsSettings["data_path"] >> data_path;
	start_time = fsSettings["start_time"];
	data_type  = fsSettings["data_type"];
	should_resize = fsSettings["resize"];
	pub_size.width = fsSettings["input_width"];
	pub_size.height = fsSettings["input_height"];
	input_rate = fsSettings["input_rate"];


	fsSettings.release();
}


int main(int argc, char **argv)
{
    ros::init(argc, argv, "image_publisher");
    ros::NodeHandle nh;
    image_transport::ImageTransport it(nh);//发布图片需要用到image_transport
	readParam(config_path);

	image_transport::Publisher pub = it.advertise(data_pub_topic, 100);

	int img_cnt = 0;
	int video_rate = 0;
	Mat image;

	VideoCapture cap;

    //TODO read imgs
	std::vector<std::string> files;

	if(data_type==1){
		//    std::string source = "/home/lab/Dataset/kinect/modu_768x432"; //kinect color
		if (getdir(data_path, files) >= 0) {  //如果是文件加那就读取data_path的文件夹内所有图像
			printf("found %d image files in folder %s!\n", (int) files.size(), data_path.c_str());
		} else if (getFile(data_path, files) >= 0) {
			printf("found %d image files in file %s!\n", (int) files.size(), data_path.c_str());
		} else {
			printf("could not load file list! wrong path / file?\n");
		}
		img_cnt = start_time;
	}else{
		cap.open(data_path);
		video_rate = (int)cap.get(CV_CAP_PROP_FPS);
		cout<<"start from "<<start_time * video_rate<<" frame"<<endl;
		cap.set(CAP_PROP_POS_FRAMES, start_time * video_rate);

//		int cnt = 0;
//		while(data_type == 0 && cnt < start_time * video_rate){  //丢弃
//			cnt++;
//			cap.read(image);
//		}
	}

    ros::Rate loop_rate(input_rate);  //单位Hz

    while (nh.ok())
    {
		img_cnt++;
		if (data_type==1 ){
			if(img_cnt < files.size()){
				image = cv::imread(files[img_cnt], CV_LOAD_IMAGE_UNCHANGED);
			}else break;
		}else{
			if(!cap.read(image))
				break;
		}

		if (should_resize && pub_size.width != image.cols && pub_size.height != image.rows) {
			cout << "resized: [" << image.size << "] ---> " << pub_size ;

			cv::resize(image, image, Size(pub_size.width, pub_size.height), cv::INTER_AREA);
		}

        cout<<"---------------frame: "<<img_cnt<<endl;
        std_msgs::Header frameheader;

//		imshow("image", image);
//		waitKey(1);

        frameheader.stamp.fromNSec((1.0f*float(img_cnt)*1e9) / (float)input_rate);
        frameheader.frame_id = to_string(frameheader.stamp.sec) + "." + to_string(frameheader.stamp.nsec);

//        cout<<"frameheader.stamp "<<frameheader.stamp <<endl;
//
//        cout<<"frameheader.stamp.sec  "<<frameheader.stamp.sec <<endl;
//        cout<<"frameheader.stamp.nsec "<<frameheader.stamp.nsec<<endl;
//
//        cout<<"frameheader.frame_id   "<<frameheader.frame_id <<endl;
//        cout<<"frameheader.stamp.toSec() :  "<<frameheader.stamp.toSec() <<endl;

        sensor_msgs::ImagePtr msg = cv_bridge::CvImage(frameheader, "bgr8", image).toImageMsg();

        pub.publish(msg);

        ros::spinOnce();

		loop_rate.sleep();
    }

}


