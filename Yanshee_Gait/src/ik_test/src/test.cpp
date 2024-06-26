#include "ros/ros.h"
#include "gait/kinematics.h"
#include "ubt_msgs/angles_set.h"
#include <sstream>
#include "ubt_msgs/servo_write_1.h"
#include "ubt_msgs/servo_write_list.h"


#define LEG_UP_CONDITION 0.006
#define WAIST_LIMIT 0.007
#define GAIT_T 0.8
#define SPEED 0.01

enum gait_status
{
    LEFT_UP = 0,
    WAIST_TO_LEFT = 1,
    RIGHT_UP = 2,
    WAIST_TO_RIGHT = 3,
};


// enum servo_read_req {
//         RightShoulderRoll=0,
//         RightElbowFlex=1,
//         RightShoulderFlex,
//         LeftShoulderRoll,
//         LeftShoulderFlex,
//         LeftElbowFlex,
//         RightHipLR,
//         RightHipFB,
//         RightKneeFlex,
//         RightAnkleFB,
//         RightAnkleUD,
//         LeftHipLR,
//         LeftHipFB,
//         LeftKneeFlex,
//         LeftAnkleFB,
//         LeftAnkleUD,
//         NeckLR
// };
 std::string robot_state[17]  = {
        "RightShoulderRoll", "RightElbowFlex", "RightShoulderFlex",
        "LeftShoulderRoll", "LeftShoulderFlex", "LeftElbowFlex",
        "RightHipLR", "RightHipFB", "RightKneeFlex",
        "RightAnkleFB", "RightAnkleUD",
        "LeftHipLR", "LeftHipFB", "LeftKneeFlex",
        "LeftAnkleFB", "LeftAnkleUD",
        "NeckLR"
    };

gait_status gait_current_status;
bool first_step = true;

int main(int argc, char **argv)
{
  
  ros::init(argc, argv, "ik_test_node");
  ros::NodeHandle n;
  
  ros::Publisher joint_pub = n.advertise<ubt_msgs::angles_set>("hal_angles_set", 1);
  ros::Publisher servo_pub = n.advertise<ubt_msgs::servo_write_list>("/hal_servo_write", 1);
  ubt_msgs::angles_set joint_angle_;
  joint_angle_.angles.resize(17);
  
  gait::Kinematics kinematic;
  ros::Rate loop_rate(50);
  ros::Time time_program_hold = ros::Time::now();
  
  const double PI = 3.141526;
  const double pi = 3.141526;
  const double Rad2Deg = 180 / pi;
  const double Deg2Rad = pi / 180;

  double l[6] = {0.0, 0.0563/2, 0.0, 0.0, 0.0, 0.0};
  double r[6] = {0.0, -0.0563/2, 0.0, 0.0, 0.0, 0.0};
  double w[6] = {0.015, 0.0, 0.152776, 0.0, 0.0, 0.0};
  
  double foot_pos_l[3] = {0,0,0};
  double foot_pos_r[3] = {0,0,0};
  
  double stand_init_w[6] = {0, 0, 0.15, 0, 0, 0};
  
  double left_arm[3] = {0,0,0};
  double right_arm[3] = {0,0,0};
  
  double l_foot[6] = {0, 0.0563/2, 0, 0, 0, 0};
  double r_foot[6] = {0, -0.0563/2, 0, 0, 0, 0};
  double waist[6] = {0, 0, 0.15, 0, 0, 0};
  double l_joint[6] = {0, 0, 0, 0, 0, 0};
  double r_joint[6] = {0, 0, 0, 0, 0, 0};
  //init joint_angle
  joint_angle_.angles[0] = 90;
  joint_angle_.angles[1] = 130;
  joint_angle_.angles[2] = 179;
  joint_angle_.angles[3] = 90;
  joint_angle_.angles[4] = 40;
  joint_angle_.angles[5] = 15;
  joint_angle_.angles[6] = 90;
  joint_angle_.angles[7] = 60;
  joint_angle_.angles[8] = 76;
  joint_angle_.angles[9] = 110;
  joint_angle_.angles[10] = 90;
  joint_angle_.angles[11] = 90;
  joint_angle_.angles[12] = 120;
  joint_angle_.angles[13] = 104;
  joint_angle_.angles[14] = 70;
  joint_angle_.angles[15] = 90;
  joint_angle_.angles[16] = 90;
  joint_angle_.time = 25;
  while (ros::ok())
  {
    ros::Duration ros_time_dur = ros::Time::now() - time_program_hold;
	double time_now = ros_time_dur.toSec();//update run time
	//set robot waist position
	waist[0] = stand_init_w[0] +(l_foot[0]+r_foot[0])/2;
	waist[1] = stand_init_w[1] + WAIST_LIMIT*(sin(2* PI/GAIT_T * time_now));
	waist[2] = stand_init_w[2];
	waist[3] = 0;
	waist[4] = 0;
	waist[5] = 0;
	//set robot foot position
	if(waist[1]>LEG_UP_CONDITION)
     {
	    gait_current_status = LEFT_UP;				    
	 }else if(waist[1]<-LEG_UP_CONDITION)
	 {
		gait_current_status = RIGHT_UP;
		first_step = false;
	 }else
	 {
	    if((gait_current_status == LEFT_UP)||(gait_current_status == WAIST_TO_LEFT))
		{
			gait_current_status = WAIST_TO_LEFT	;					    
		}else
		{
			gait_current_status = WAIST_TO_RIGHT;	
		}
	 }
				
    switch(gait_current_status)
      {
		case LEFT_UP:
		    foot_pos_l[2] = 15*(waist[1]-LEG_UP_CONDITION);
			foot_pos_l[0] += (first_step?(SPEED/2):SPEED);
			// left_arm[0] = 30000*(waist[1]-LEG_UP_CONDITION);
			// right_arm[0] = left_arm[0];
			break;
		case WAIST_TO_LEFT:
			foot_pos_l[2] = 0;
			foot_pos_r[2] = 0;
			break;
		case RIGHT_UP:
			foot_pos_r[2] = -15*(waist[1]+LEG_UP_CONDITION);	
			foot_pos_r[0] += (first_step?(SPEED/2):SPEED);
			// left_arm[0] = 30000*(waist[1]+LEG_UP_CONDITION);
			// right_arm[0] = left_arm[0];
			break;
		case WAIST_TO_RIGHT:
			foot_pos_l[2] = 0;
			foot_pos_r[2] = 0;
			break;
	   }	
	
	l_foot[0] = l[0] + foot_pos_l[0];
    l_foot[1] = l[1] + foot_pos_l[1];
    l_foot[2] = l[2] + foot_pos_l[2];

    r_foot[0] = r[0] + foot_pos_r[0];
    r_foot[1] = r[1] + foot_pos_r[1];
    r_foot[2] = r[2] + foot_pos_r[2]; 
    //Calculating angle value use IK API
	kinematic.IK_leg(l_foot, waist, r_foot, l_joint, r_joint);
    std::cout <<"l_joint="<<  l_joint[0] << ' ' << l_joint[1] << ' ' << l_joint[2] << ' ' << l_joint[3] << ' ' << l_joint[4] << ' ' << l_joint[5] << std::endl;
    std::cout <<"r_joint="<< r_joint[0] << ' ' << r_joint[1] << ' ' << r_joint[2] << ' ' << r_joint[3] << ' ' << r_joint[4] << ' ' << r_joint[5] << std::endl;
    std::cout <<"l_foot="<<  l_foot[0] << ' ' << l_foot[1] << ' ' << l_foot[2] << ' ' << l_foot[3] << ' ' << l_foot[4] << ' ' << l_foot[5] << std::endl;
    std::cout <<"r_foot="<<  r_foot[0] << ' ' << r_foot[1] << ' ' << r_foot[2] << ' ' << r_foot[3] << ' ' << r_foot[4] << ' ' << r_foot[5] << std::endl;
	// joint_angle_.angles[0] = int((90 + left_arm[0]));
    // joint_angle_.angles[3] = int((90 + right_arm[0]));
	
	// joint_angle_.angles[6] = int((90 + 1*l_joint[1]*Rad2Deg ));
    joint_angle_.angles[7] = int((60 + 1*l_joint[2]*Rad2Deg ));
    joint_angle_.angles[8] = int((76 - 1*l_joint[3]*Rad2Deg ));
    joint_angle_.angles[9] = int((110 - 1*l_joint[4]*Rad2Deg ));
    joint_angle_.angles[10] = int((90 + 1*l_joint[5]*Rad2Deg ));
        
    // joint_angle_.angles[11] = int((90 + 1*r_joint[1]*Rad2Deg ));
    joint_angle_.angles[12] = int((120 - 1*r_joint[2]*Rad2Deg ));
    joint_angle_.angles[13] = int((104 + 1*r_joint[3]*Rad2Deg ));
    joint_angle_.angles[14] = int((70 + 1*r_joint[4]*Rad2Deg ));
    joint_angle_.angles[15] = int((90 + 1*r_joint[5]*Rad2Deg ));
    
	// joint_angle_.time = 20;
    // joint_pub.publish(joint_angle_);
	ubt_msgs::servo_write_1 msg_1;
    ubt_msgs::servo_write_list msg_2;
    
    for(int i=0;i<17;i++)
    {
        msg_1.name = robot_state[i];
        msg_1.angle = joint_angle_.angles[i];
        msg_1.runtime = 20;
        msg_2.data.push_back(msg_1);
    }
    servo_pub.publish(msg_2);
	ros::spinOnce();
    loop_rate.sleep();
  }

  return 0;
}

