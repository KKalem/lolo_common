#include "ros/ros.h"
#include <stdio.h>
#include <sstream>
#include <boost/chrono.hpp>
#include <boost/thread/thread.hpp>
#include "TcpInterface.h"
#include <stdint.h>

#include "std_msgs/String.h"
#include "sensor_msgs/FluidPressure.h"
#include "sensor_msgs/MagneticField.h"
#include "sensor_msgs/Imu.h"
#include "sensor_msgs/NavSatFix.h"
#include "smarc_msgs/DVL.h"
#include "smarc_msgs/AltitudeStamped.h"
#include "smarc_msgs/CaptainStatus.h"
#include "smarc_msgs/RudderAngle.h"
#include "smarc_msgs/ThrusterRPM.h"
#include "smarc_msgs/ThrusterRPMStatus.h"
#include "smarc_msgs/Heartbeat.h"
#include "geometry_msgs/PoseWithCovarianceStamped.h"
#include "geometry_msgs/TwistWithCovarianceStamped.h"
#include "geometry_msgs/Point.h"
#include "std_msgs/Float32.h"

#include "captain_interface/scientistmsg.h"

#include <tf2/LinearMath/Quaternion.h>

#define IPADDRESS "192.168.1.177" // IP address of LOLO
#define PORT 8888


TcpInterFace captain;

struct ROSinterface {

  //================== ROS Nodehandle ====================//
  ros::NodeHandle* n;

  //======================================================//
  //================== ROS subscribers ===================//
  //======================================================//

  //TODO
  // USBL/RF/etc

  // course
  // pitch
  // depth
  // altitude
  // waypoint

  //information / other things
  ros::Subscriber heartbeat_sub;        //heartbeat message
  ros::Subscriber done_sub;             //scientist mission completed
  ros::Subscriber abort_sub;            //abort

  //Control commands: High level
  ros::Subscriber waypoint_sub;         //target yaw
  ros::Subscriber speed_sub;            //target speed
  ros::Subscriber depth_sub;            //target depth
  ros::Subscriber altitude_sub;         //target altitude

  //Control commands medium level
  ros::Subscriber yaw_sub;              //target course
  ros::Subscriber yawrate_sub;          //target turn rate
  ros::Subscriber pitch_sub;            //target pitch
  ros::Subscriber rpm_sub;              //target RPM (average if differential thrust is enabled)

  //Control commands low level
  //thrusters
  ros::Subscriber thrusterPort_sub;
  ros::Subscriber thrusterStrb_sub;

  //control surfaces
  ros::Subscriber rudderPort_sub;
  ros::Subscriber rudderStrb_sub;
  ros::Subscriber elevator_sub;

  //======================================================//
  //=================== ROS pubishers ====================//
  //======================================================//
  //thrusters
  ros::Publisher thrusterPort_pub;
  ros::Publisher thrusterStrb_pub;

  //constrol surfaces
  ros::Publisher rudderPort_pub;
  ros::Publisher rudderStrb_pub;
  ros::Publisher elevator_pub;

  //sensors
  ros::Publisher imu_pub;
  ros::Publisher magnetometer_pub;
  ros::Publisher pressure_pub;
  ros::Publisher dvl_pub;
  ros::Publisher gps_pub;

  //control / status
  ros::Publisher status_altitude_pub;
  ros::Publisher status_position_pub;
  ros::Publisher status_twist_pub;

  ros::Publisher control_status_pub;

  //General purpose text output
  ros::Publisher text_pub;

  //======================================================//
  //=================== ROS callbacks ====================//
  //======================================================//

  void callback_heartbeat(const smarc_msgs::Heartbeat::ConstPtr &_msg) {
    captain.new_package(SC_HEARTBEAT); // Heartbeat message
    captain.send_package();
  };

  //void callback_done(const smarc_msgs::done::ConstPtr &_msg) {};

  //void callback_abort(const smarc_msgs::abort::ConstPtr &_msg) {};

  void callback_waypoint(const geometry_msgs::Point::ConstPtr &_msg) {
    float lat = _msg->x;
    float lon = _msg->y;
    captain.new_package(SC_SET_TARGET_WAYPOINT); // set target waypoint
    captain.add_float(lat);
    captain.add_float(lon);
    captain.send_package();
  };

  void callback_speed(const std_msgs::Float32::ConstPtr &_msg) {
    float targetSpeed = _msg->data;
    captain.new_package(SC_SET_TARGET_SPEED); // set target speed
    captain.add_float(targetSpeed);
    captain.send_package();
  };

  void callback_depth(const std_msgs::Float32::ConstPtr &_msg) {
    float targetDepth = _msg->data;
    captain.new_package(SC_SET_TARGET_DEPTH); // set target depth
    captain.add_float(targetDepth);
    captain.send_package();
  };

  void callback_altitude(const std_msgs::Float32::ConstPtr &_msg) {
    float targetAltitude = _msg->data;
    captain.new_package(SC_SET_TARGET_ALTITUDE); // set target altitude
    captain.add_float(targetAltitude);
    captain.send_package();
  };

  void callback_yaw(const std_msgs::Float32::ConstPtr &_msg) {
    float targetYaw = _msg->data;
    captain.new_package(SC_SET_TARGET_YAW); // set target yaw
    captain.add_float(targetYaw);
    captain.send_package();
  };

  void callback_yawrate(const std_msgs::Float32::ConstPtr &_msg) {
    float targetYawRate = _msg->data;
    captain.new_package(SC_SET_TARGET_YAW_RATE); // set target yaw rate
    captain.add_float(targetYawRate);
    captain.send_package();
  };

  void callback_pitch(const std_msgs::Float32::ConstPtr &_msg) {
    float targetPitch = _msg->data;
    captain.new_package(SC_SET_TARGET_PITCH); // set target pitch
    captain.add_float(targetPitch);
    captain.send_package();
  };

  void callback_rpm(const smarc_msgs::ThrusterRPM::ConstPtr &_msg) {
    float targetRPM = _msg->RPM;
    captain.new_package(SC_SET_TARGET_RPM); // targetRPM
    captain.add_float(targetRPM);
    captain.send_package();
  };


  void callback_rudderPort(const smarc_msgs::RudderAngle::ConstPtr &_msg) {
    //printf("Port rudder: %f\n", _msg->angle);
    captain.new_package(SC_SET_RUDDER_PORT); // Control message
    captain.add_float(_msg->angle);
    captain.send_package();
  };

  void callback_rudderStrb(const smarc_msgs::RudderAngle::ConstPtr &_msg) {
    //printf("Strb rudder: %f\n", _msg->angle);
    captain.new_package(SC_SET_RUDDER_STRB);
    captain.add_float(_msg->angle);
    captain.send_package();
  };

  void callback_elevator(const smarc_msgs::RudderAngle::ConstPtr &_msg) {
    //printf("elevator: %f\n", _msg->angle);
    captain.new_package(SC_SET_ELEVATOR);
    captain.add_float(_msg->angle);
    captain.send_package();
  };

  void callback_thrusterPort(const smarc_msgs::ThrusterRPM::ConstPtr &_msg) {
    //printf("Port Thruster angle received %f\n", _msg->angle);
    captain.new_package(SC_SET_THRUSTER_PORT);
    captain.add_float(_msg->RPM);
    captain.send_package();
  };

  void callback_thrusterStrb(const smarc_msgs::ThrusterRPM::ConstPtr &_msg) {
    //printf("Strb Thruster: %f\n", _msg->angle);
    captain.new_package(SC_SET_THRUSTER_STRB);
    captain.add_float(_msg->RPM);
    captain.send_package();
  };


  void init(ros::NodeHandle* nh) {
    n = nh;
    //==================================//
    //=========== Subscribers ==========//
    //==================================//

    //information / other things
    heartbeat_sub  = n->subscribe<smarc_msgs::Heartbeat>("/test/Heartbeat", 1, &ROSinterface::callback_heartbeat, this);
    //done_sub  = n->subscribe<s
    //abort_sub  = n->subscribe<

    //Control commands: High level
    waypoint_sub  = n->subscribe<geometry_msgs::Point>("/test/setwaypoint", 1, &ROSinterface::callback_waypoint, this);
    speed_sub     = n->subscribe<std_msgs::Float32>("/test/setspeed",1,&ROSinterface::callback_speed,this);
    depth_sub     = n->subscribe<std_msgs::Float32>("/test/setdepth",1,&ROSinterface::callback_depth,this);
    altitude_sub  = n->subscribe<std_msgs::Float32>("/test/setaltitude",1,&ROSinterface::callback_altitude,this);

    //Control commands medium level
    yaw_sub       = n->subscribe<std_msgs::Float32>("/test/setyaw",1,&ROSinterface::callback_yaw,this);
    yawrate_sub   = n->subscribe<std_msgs::Float32>("/test/setyawrate",1,&ROSinterface::callback_yawrate,this);
    pitch_sub     = n->subscribe<std_msgs::Float32>("/test/setpitch",1,&ROSinterface::callback_pitch,this);
    rpm_sub       = n->subscribe<smarc_msgs::ThrusterRPM>("/test/setrpm", 1, &ROSinterface::callback_rpm, this);

    //Control commands low level
    //Thruster
    thrusterPort_sub = n->subscribe<smarc_msgs::ThrusterRPM>("/lolo_auv_1/thrusters/0/input", 1, &ROSinterface::callback_thrusterPort, this);
    thrusterStrb_sub = n->subscribe<smarc_msgs::ThrusterRPM>("/lolo_auv_1/thrusters/1/input", 1, &ROSinterface::callback_thrusterStrb, this);

    //control surfaces
    rudderPort_sub  = n->subscribe<smarc_msgs::RudderAngle>("/lolo_auv_1/fins/0/input", 1, &ROSinterface::callback_rudderPort, this);
    rudderStrb_sub  = n->subscribe<smarc_msgs::RudderAngle>("/lolo_auv_1/fins/1/input", 1, &ROSinterface::callback_rudderStrb, this);
    elevator_sub    = n->subscribe<smarc_msgs::RudderAngle>("/lolo_auv_1/back_fins/0/input", 1, &ROSinterface::callback_elevator, this);

    //==================================//
    //=========== Publishers ===========//
    //==================================//
    //thrusters
    thrusterPort_pub     = n->advertise<smarc_msgs::ThrusterRPMStatus>("/lolo_auv_1/thrusters/0/output", 10);
    thrusterStrb_pub     = n->advertise<smarc_msgs::ThrusterRPMStatus>("/lolo_auv_1/thrusters/1/output", 10);

    //constrol surfaces
    rudderPort_pub    = n->advertise<smarc_msgs::RudderAngle>("/lolo_auv_1/fins/0/output", 10);
    rudderStrb_pub    = n->advertise<smarc_msgs::RudderAngle>("/lolo_auv_1/fins/1/output", 10);
    elevator_pub      = n->advertise<smarc_msgs::RudderAngle>("/lolo_auv_1/back_fins/0/output", 10);

    //sensors
    imu_pub           = n->advertise<sensor_msgs::Imu>("/test/imu", 10);
    magnetometer_pub  = n->advertise<sensor_msgs::MagneticField>("/test/mag", 10);
    dvl_pub           = n->advertise<smarc_msgs::DVL>("/test/dvl", 10);
    gps_pub           = n->advertise<sensor_msgs::NavSatFix>("/test/gps", 10);
    pressure_pub      = n->advertise<sensor_msgs::FluidPressure>("/test/pressure", 10);

    //control / status
    status_altitude_pub = n->advertise<smarc_msgs::AltitudeStamped>("/test/status/altitude", 10);
    status_position_pub = n->advertise<geometry_msgs::PoseWithCovarianceStamped>("/test/status/position",10);
    status_twist_pub    = n->advertise<geometry_msgs::TwistWithCovarianceStamped>("test/status/twist",10);
    control_status_pub  = n->advertise<smarc_msgs::CaptainStatus>("/test/control_status", 10);

    //General purpose text message
    text_pub   = n->advertise<std_msgs::String>("/test/text", 10);
  }
};

ROSinterface rosInterface;

//======================================================//
//========= Captain interface callback =================//
//======================================================//
int captain_msgs = 0;
void callback_captain() {
  int msgID = captain.messageID();
  //printf("Received message from captain %d\n", msgID);
  switch (msgID) {
    case CS_STATUS: { //status
      uint64_t timestamp    = captain.parse_llong();
      uint64_t sec = timestamp / 1000000;
      uint64_t usec = timestamp % 1000000;
      uint32_t sequence     = captain.parse_long();

      float lat         = captain.parse_float();
      float lon         = captain.parse_float();
      float depth       = captain.parse_float();
      float altitude    = captain.parse_float();
      float sogX        = captain.parse_float();
      float sogY        = captain.parse_float();
      float sogZ        = captain.parse_float();
      float rotX        = captain.parse_float();
      float rotY        = captain.parse_float();
      float rotZ        = captain.parse_float();
      float pitch       = captain.parse_float();
      float roll        = captain.parse_float();
      float yaw         = captain.parse_float();

      //publish position
      geometry_msgs::PoseWithCovarianceStamped pos_msg;
      pos_msg.header.stamp = ros::Time(sec,usec*1000);
      pos_msg.header.seq = sequence;
      pos_msg.pose.pose.position.x = lat;
      pos_msg.pose.pose.position.y = lon;
      pos_msg.pose.pose.position.z = depth;

      tf2::Quaternion q; q.setRPY(roll, pitch, yaw); q.normalize();
      pos_msg.pose.pose.orientation.x = q[0];
      pos_msg.pose.pose.orientation.y = q[1];
      pos_msg.pose.pose.orientation.z = q[2];
      pos_msg.pose.pose.orientation.w = q[3];

      rosInterface.status_position_pub.publish(pos_msg);

      geometry_msgs::TwistWithCovarianceStamped twist_msg;
      twist_msg.header.stamp = ros::Time(sec,usec*1000);
      twist_msg.header.seq = sequence;
      twist_msg.twist.twist.linear.x = sogX;
      twist_msg.twist.twist.linear.y = sogY;
      twist_msg.twist.twist.linear.z = sogZ;
      twist_msg.twist.twist.angular.x = rotX;
      twist_msg.twist.twist.angular.y = rotY;
      twist_msg.twist.twist.angular.z = rotZ;
      rosInterface.status_twist_pub.publish(twist_msg);

      //publish altitude
      smarc_msgs::AltitudeStamped alt_msg;
      alt_msg.header.stamp = ros::Time(sec,usec*1000);
      alt_msg.header.seq = sequence;
      alt_msg.altitude = altitude;
      rosInterface.status_altitude_pub.publish(alt_msg);
    }
    break;
    case CS_CONTROL: { //control
      uint64_t timestamp    = captain.parse_llong();
      uint64_t sec = timestamp / 1000000;
      uint64_t usec = timestamp % 1000000;
      uint32_t sequence     = captain.parse_long();

      float source  = captain.parse_byte();
      float targetWaypoint_lat = captain.parse_float();
      float targetWaypoint_lon =  captain.parse_float();
      float targetYaw      = captain.parse_float();
      float targetPitch    = captain.parse_float();
      float targetSpeed    = captain.parse_float();
      float targetRPM      = captain.parse_float();
      float targetDepth    = captain.parse_float();
      float targetAltitude = captain.parse_float();

      smarc_msgs::CaptainStatus msg;
      msg.header.stamp = ros::Time(sec,usec*1000);
      msg.header.seq = sequence;
      msg.active_control_input = source;
      rosInterface.control_status_pub.publish(msg);
    }
    break;
    case CS_RUDDER_PORT: { //port rudder
      uint64_t timestamp    = captain.parse_llong();
      uint32_t sequence     = captain.parse_long();
      float target_angle    = captain.parse_float();
      float current_angle   = captain.parse_float();

      smarc_msgs::RudderAngle msg;
      uint64_t sec = timestamp / 1000000;
      uint64_t usec = timestamp % 1000000;
      msg.header.stamp = ros::Time(sec,usec*1000);
      msg.header.seq = sequence;
      msg.angle = current_angle;
      rosInterface.rudderPort_pub.publish(msg);
    }
    break;
    case CS_RUDDER_STRB: { //strb rudder
      uint64_t timestamp    = captain.parse_llong();
      uint32_t sequence     = captain.parse_long();
      float target_angle    = captain.parse_float();
      float current_angle   = captain.parse_float();

      smarc_msgs::RudderAngle msg;
      uint64_t sec = timestamp / 1000000;
      uint64_t usec = timestamp % 1000000;
      msg.header.stamp = ros::Time(sec,usec*1000);
      msg.header.seq = sequence;
      msg.angle = current_angle;
      rosInterface.rudderStrb_pub.publish(msg);
    }
    break;
    case CS_ELEVATOR: { //elevator
      uint64_t timestamp    = captain.parse_llong();
      uint32_t sequence     = captain.parse_long();
      float target_angle    = captain.parse_float();
      float current_angle   = captain.parse_float();

      smarc_msgs::RudderAngle msg;
      uint64_t sec = timestamp / 1000000;
      uint64_t usec = timestamp % 1000000;
      msg.header.stamp = ros::Time(sec,usec*1000);
      msg.header.seq = sequence;
      msg.angle = current_angle;
      rosInterface.elevator_pub.publish(msg);
    }
    break;
    case CS_THRUSTER_PORT: { //port thruster
      uint64_t timestamp    = captain.parse_llong();
      uint32_t sequence     = captain.parse_long();
      uint64_t sec = timestamp / 1000000;
      uint64_t usec = timestamp % 1000000;

      float rpm             = captain.parse_float();
      float current         = captain.parse_float();
      float torque          = captain.parse_float();
      float energy          = captain.parse_float();
      float voltage         = captain.parse_float();

      smarc_msgs::ThrusterRPMStatus msg;
      msg.thrusterRPM.header.stamp = ros::Time(sec,usec*1000);
      msg.thrusterRPM.header.seq = sequence;
      msg.thrusterRPM.RPM = rpm;
      msg.current = current;
      msg.torque = torque;
      msg.temperature = 0;
      rosInterface.thrusterPort_pub.publish(msg);
    }
    break;
    case CS_THRUSTER_STRB: { //strb thruster
      uint64_t timestamp    = captain.parse_llong();
      uint32_t sequence     = captain.parse_long();
      uint64_t sec = timestamp / 1000000;
      uint64_t usec = timestamp % 1000000;

      float rpm             = captain.parse_float();
      float current         = captain.parse_float();
      float torque          = captain.parse_float();
      float energy          = captain.parse_float();
      float voltage         = captain.parse_float();

      smarc_msgs::ThrusterRPMStatus msg;
      msg.thrusterRPM.header.stamp = ros::Time(sec,usec*1000);
      msg.thrusterRPM.header.seq = sequence;
      msg.thrusterRPM.RPM = rpm;
      msg.current = current;
      msg.torque = torque;
      msg.temperature = 0;
      rosInterface.thrusterStrb_pub.publish(msg);
    }
    break;
    case CS_BATTERY: { //battery
      //TODO add something
    }
    break;
    case CS_DVL: { //DVL
      uint64_t timestamp    = captain.parse_llong();
      uint64_t sec = timestamp / 1000000;
      uint64_t usec = timestamp % 1000000;
      uint32_t sequence     = captain.parse_long();

      float sogX           = captain.parse_float();
      float sogY           = captain.parse_float();
      float sogZ           = captain.parse_float();
      float stwX           = captain.parse_float();
      float stwY           = captain.parse_float();
      float stwZ           = captain.parse_float();
      float range1         = captain.parse_float();
      float range2         = captain.parse_float();
      float range3         = captain.parse_float();
      float range4         = captain.parse_float();
      float range = 0.25*(range1+range2+range3+range4);

      smarc_msgs::DVL msg;
      msg.header.stamp = ros::Time(sec,usec*1000);
      msg.header.seq = sequence;
      msg.velocity.x = sogX;
      msg.velocity.y = sogY;
      msg.velocity.z = sogZ;
      msg.velocity_reference = smarc_msgs::DVL::VELOCITY_REFERENCE_BOTTOM;
      msg.range = range;
      rosInterface.dvl_pub.publish(msg);
    }
    break;
    case CS_GPS: { //GPS
      uint64_t timestamp    = captain.parse_llong();
      uint64_t sec = timestamp / 1000000;
      uint64_t usec = timestamp % 1000000;
      uint32_t sequence     = captain.parse_long();

      float lat           = captain.parse_float();
      float lon           = captain.parse_float();
      float cog           = captain.parse_float();
      float sog           = captain.parse_float();

      sensor_msgs::NavSatFix msg;
      msg.header.stamp = ros::Time(sec,usec*1000);
      msg.header.seq = sequence;
      msg.latitude = lat;
      msg.longitude = lon;
      rosInterface.gps_pub.publish(msg);
    }
    break;
    case CS_IMU: { //IMU
      uint64_t timestamp    = captain.parse_llong();
      uint64_t sec = timestamp / 1000000;
      uint64_t usec = timestamp % 1000000;
      uint32_t sequence     = captain.parse_long();

      float pitch           = captain.parse_float();
      float roll            = captain.parse_float();
      float yaw             = captain.parse_float();

      float accX            = captain.parse_float();
      float accY            = captain.parse_float();
      float accZ            = captain.parse_float();

      float rotX            = captain.parse_float();
      float rotY            = captain.parse_float();
      float rotZ            = captain.parse_float();

      sensor_msgs::Imu msg;
      msg.header.stamp = ros::Time(sec,usec*1000);
      msg.header.seq = sequence;

      tf2::Quaternion q; q.setRPY(roll, pitch, yaw); q.normalize();
      msg.orientation.x = q[0];
      msg.orientation.y = q[1];
      msg.orientation.z = q[2];
      msg.orientation.w = q[3];

      msg.angular_velocity.x = rotX;
      msg.angular_velocity.y = rotY;
      msg.angular_velocity.z = rotZ;

      msg.linear_acceleration.x = accX;
      msg.linear_acceleration.y = accY;
      msg.linear_acceleration.z = accZ;

      rosInterface.imu_pub.publish(msg);
    }
    break;
    case CS_MAG: { //MAG
      uint64_t timestamp    = captain.parse_llong();
      uint64_t sec = timestamp / 1000000;
      uint64_t usec = timestamp % 1000000;
      uint32_t sequence     = captain.parse_long();
      float magX            = captain.parse_float();
      float magY            = captain.parse_float();
      float magZ            = captain.parse_float();

      sensor_msgs::MagneticField msg;
      msg.header.stamp = ros::Time(sec,usec*1000);
      msg.header.seq = sequence;
      msg.magnetic_field.x = magX;
      msg.magnetic_field.y = magY;
      msg.magnetic_field.z = magZ;
      //msg.magnetic_field_covariance = 0;
      rosInterface.magnetometer_pub.publish(msg);
    }
    break;
    case CS_PRESSURE: { //PRESSURE
      uint64_t timestamp    = captain.parse_llong();
      uint64_t sec = timestamp / 1000000;
      uint64_t usec = timestamp % 1000000;
      uint32_t sequence     = captain.parse_long();
      float pressure        = captain.parse_float();

      sensor_msgs::FluidPressure msg;
      msg.header.stamp = ros::Time(sec,usec*1000);
      msg.header.seq = sequence;
      msg.fluid_pressure = pressure;
      msg.variance = 0;
      rosInterface.pressure_pub.publish(msg);
    }
    break;
    case CS_TEXT: { //General purpose text message
      int length = captain.parse_byte();
      String text = captain.parse_string(length);
      std_msgs::String msg;
      msg.data = text.c_str();
      rosInterface.text_pub.publish(msg);
    }
    break;
  }
}


int main(int argc, char *argv[]) {

  printf("Set callback\n");
  captain.setCallback(callback_captain);

  printf("setup\n");
  captain.setup(IPADDRESS, PORT); //Dont start until captain is connected and running.

  if(captain.connected())
    printf("main::Connected\n");
  else {
    printf("main::Not connceted\n");
    //return 0;
  }

  printf("main::ros init\n");

  ros::init(argc,argv, "CaptainInterface");

  ros::NodeHandle n;
  //Init subscribers and publishers
  rosInterface.init(&n);

  printf("Loop starting\n");
  ros::Rate loop_rate(1000);
  while(ros::ok()) {
    captain.loop(); //should be improved.
    ros::spinOnce();
  }
}
