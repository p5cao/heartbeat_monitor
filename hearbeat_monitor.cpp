# include <rclcpp/rclcpp.hpp>
# include "std_msgs/msg/bool.hpp"

# include <chrono>


using namespace std::chrono_literals;
using namespace std::placeholders;


class HeartbeatMonitorNode : public rclcpp::Node 
{
    public:
    
        HeartbeatMonitorNode():
        Node("hearbeat_monitor_node"),
        has_received_heartbeat_(false),
        system_alive_(false)
        {
            //create subscription to /heartbeat topic
            heartbeat_subscriber_ = this->create_subscription<std_msgs::msg::Bool>(
                "/heartbeat",
                rclcpp::QoS(10),
                std::bind(&HeartbeatMonitorNode::heartbeatCallback, this, _1)
                
            );

            //create publisher to /system_alive topic
            system_alive_publisher_ = this->create_publisher<std_msgs::msg::Bool>(
                "/system_alive",
                rclcpp::QoS(10)
            );

            timer_ = this->create_wall_timer(
                100ms,
                std::bind(&HeartbeatMonitorNode::walltimerCallback, this)
            );

            RCLCPP_INFO(this->get_logger(), "Heartbeat monitor node started.");
        }



    private:

        rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr heartbeat_subscriber_;
        rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr system_alive_publisher_;
        rclcpp::TimerBase::SharedPtr timer_;

        rclcpp::Time last_heartbeat_time_;
        double timeout_sec_ = 0.5;
        bool has_received_heartbeat_;
        bool system_alive_;


        void heartbeatCallback(const std_msgs::msg::Bool::SharedPtr msg){
            if(msg->data)
            {
                last_heartbeat_time_ = this->now();
                has_received_heartbeat_ = true;

                RCLCPP_INFO(this->get_logger(), "Heartbeat recived.");
            }
        }

        void walltimerCallback(){

            system_alive_ = false; // by default

            if (has_received_heartbeat_){
                rclcpp::Duration elapsed = this->now() - last_heartbeat_time_;

                if (elapsed.seconds() <= timeout_sec_){
                    system_alive_ = true;
                }
            }

            if (system_alive_){
                RCLCPP_INFO(this->get_logger(), "System is ALIVE.");
            }
            else
            {
                RCLCPP_WARN(this->get_logger(), "Heartbeat timeout. System is NOT ALIVE.");
            }
            //message assign value and publish template
            std_msgs::msg::Bool alive_msg_;
            alive_msg_.data = system_alive_;
            system_alive_publisher_->publish(alive_msg_);

        }

};


int main (int argc, char **argv){
    rclcpp::init(argc, argv);
    auto node = std::make_shared<HeartbeatMonitorNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
