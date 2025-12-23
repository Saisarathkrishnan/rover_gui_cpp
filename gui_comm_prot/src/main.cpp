#include <rclcpp/rclcpp.hpp>
#include <custom_msgs/msg/marker_tag.hpp>
#include <custom_msgs/msg/gps_details.hpp>
#include <custom_msgs/msg/planner_status.hpp>
#include <custom_msgs/msg/imu_data.hpp>
#include <custom_msgs/msg/gui_command.hpp>
#include <std_msgs/msg/bool.hpp>
#include <map>

#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <arpa/inet.h>

class gui_comm : public rclcpp::Node
{
public:
    gui_comm() : Node("gui_comm")
    {
        std::string ip = "127.0.0.1";
        std::cout << "enter ip press q for default" << ":";
        std::cin >> ip;
        if (ip == "q")
        {
            ip = "127.0.0.1";
        }
        sock = socket(AF_INET, SOCK_STREAM, 0);

        if (sock < 0)
        {
            perror("socket");
        }

        sockaddr_in server{};
        server.sin_family = AF_INET;
        server.sin_port = htons(9000);
        inet_pton(AF_INET, ip.c_str(), &server.sin_addr);

        if (connect(sock, (sockaddr *)&server, sizeof(server)) < 0)
        {
            perror("connect");
        }

        std::cout << "hellow" << std::endl;
        std::string marker_topic = "/marker_detect";
        std::string imu_topic = "/imu_data";
        std::string gps_details_topic = "/gps_details";
        std::string planner_status_topic = "/planner/status";

        marker = this->create_subscription<custom_msgs::msg::MarkerTag>(marker_topic, 10, std::bind(&gui_comm::MarkerTag_callback, this, std::placeholders::_1));
        gps = this->create_subscription<custom_msgs::msg::GpsDetails>(gps_details_topic, 10, std::bind(&gui_comm::gps_callback, this, std::placeholders::_1));
        imu = this->create_subscription<custom_msgs::msg::ImuData>(imu_topic, 10, std::bind(&gui_comm::imu_callback, this, std::placeholders::_1));
        plan = this->create_subscription<custom_msgs::msg::PlannerStatus>(planner_status_topic, 10, std::bind(&gui_comm::planner_callback, this, std::placeholders::_1));

        guicmds_pub = this->create_publisher<custom_msgs::msg::GuiCommand>("/gui/command", 1);
        navSwitchCmd_pub = this->create_publisher<std_msgs::msg::Bool>("/autonomous_mode_cmd", 1);

        timer_ = this->create_wall_timer(std::chrono::milliseconds(100), std::bind(&gui_comm::timerCallback, this));

        rclcpp::sleep_for(std::chrono::milliseconds(200));

        std_msgs::msg::Bool lmao;
        lmao.data = false;
        navSwitchCmd_pub->publish(lmao);

        data_map = {
            {"imu_x", "nan"},
            {"imu_y", "nan"},
            {"imu_z", "nan"},
            {"marker_detect_bool", "nan"},
            {"marker_detect_id", "nan"},
            {"marker_detect_x", "nan"},
            {"marker_detect_y", "nan"},
            {"autostate_curr", "nan"},
            {"gps_lat_curr", "nan"},
            {"gps_lon_curr", "nan"},
            {"gps_alt_curr", "nan"},
            {"gps_vdops_curr", "nan"},
            {"gps_hdops_curr", "nan"},
            {"gps_fix_curr", "nan"},
            {"gps_sattelites_curr", "nan"},
            {"navmode", "nan"},
            {"destYaw", "nan"},
            {"currnYaw", "nan"},
            {"destYawError", "nan"},
            {"gpsGoal_condition", "nan"},
            {"coneGoal_condition", "nan"}};
    }

private:
    void MarkerTag_callback(const custom_msgs::msg::MarkerTag::SharedPtr markMsg)
    {
        // std::cout << markMsg->is_found << std::endl;
        (markMsg->is_found) ? data_map["marker_detect_bool"] = "TRUE" : data_map["marker_detect_bool"] = "FALSE";
        data_map["marker_detect_y"] = std::to_string(markMsg->y);
        data_map["marker_detect_x"] = std::to_string(markMsg->x);
        data_map["marker_detect_id"] = std::to_string(markMsg->id);
    }
    void gps_callback(const custom_msgs::msg::GpsDetails::SharedPtr l_msg)
    {
        data_map["gps_lat_curr"] = std::to_string(l_msg->latitude);
        data_map["gps_lon_curr"] = std::to_string(l_msg->longitude);
        data_map["gps_alt_curr"] = std::to_string(l_msg->altitude);
        data_map["gps_fix_curr"] = std::to_string(l_msg->fix_type);
        data_map["gps_sattelites_curr"] = std::to_string(l_msg->satellites);
        data_map["gps_hdops_curr"] = std::to_string(l_msg->horizontal_accuracy);
        data_map["gps_vdops_curr"] = std::to_string(l_msg->vertical_accuracy);
        // std::cout << l_msg->fix_type << std::endl;
    }
    void imu_callback(const custom_msgs::msg::ImuData::SharedPtr l_msg)
    {
        // std::cout << l_msg->orientation.x << std::endl;
        data_map["imu_z"] = std::to_string(l_msg->orientation.z);
        data_map["imu_x"] = std::to_string(l_msg->orientation.x);
        data_map["imu_y"] = std::to_string(l_msg->orientation.y);
        data_map["currnYaw"] = std::to_string(l_msg->orientation.z);
    }
    void planner_callback(const custom_msgs::msg::PlannerStatus::SharedPtr l_msg)
    {
        // std::cout << sizeof(l_msg->cone_detected) << std::endl;
        // std::cout << l_msg->cone_detected << std::endl;

        data_map["destYawError"] = std::to_string(l_msg->heading_error_deg);
        data_map["destYaw"] = std::to_string(l_msg->target_yaw_deg);
        data_map["gpsGoal_condition"] = std::to_string(l_msg->gps_goal_reached);
        data_map["coneGoal_condition"] = std::to_string(l_msg->cone_goal_reached);
        data_map["navmode"] = std::to_string(l_msg->nav_mode);
        (l_msg->autonomous_enabled) ? data_map["autostate_curr"] = "TRUE" : data_map["autostate_curr"] = "FALSE";
    }
    void timerCallback()
    {
        /*
        std::cout << "------------------------------------------" << std::endl;
        for (auto &[k, v] : data_map)
        {
            std::cout << k << " : " << v << "\n";
        }
        std::cout << "-----------------------------------------------" << std::endl;

        */
        send_111();
        receive_111();
    }

    void send_111()
    {
        nlohmann::json mj = data_map;

        std::string mj_str = mj.dump();
        uint32_t len = htonl(mj_str.size());
        send(sock, &len, sizeof(len), 0);
        send(sock, mj_str.data(), mj_str.size(), 0);
    }
    void receive_111()
    {
        uint32_t len_net = 0;
        int r = recv(sock, &len_net, sizeof(len_net), MSG_DONTWAIT);
        if (r <= 0)
        {
            return;
        }

        uint32_t len = ntohl(len_net);
        if (len == 0 || len > 1024 * 1024)
        {
            return;
        }

        std::string data(len, '\0');
        r = recv(sock, data.data(), len, MSG_DONTWAIT);
        if (r <= 0)
        {
            return;
        }

        nlohmann::json mj;
        try
        {
            mj = nlohmann::json::parse(data);
        }
        catch (const nlohmann::json::parse_error &e)
        {
            std::cerr << "JSON parse error: " << e.what() << "\n";
            std::cerr << "Raw: [" << data << "]\n";
            return;
        }
        std::cout << "gugu gaga-----------" << std::endl;
        for (auto &[key, value] : mj.items())
        {
            std::cout << key << " : " << value << std::endl;
        }
        std::cout << "gugu gaga-----------" << std::endl;

        std_msgs::msg::Bool navswitch;
        custom_msgs::msg::GuiCommand guicmdRecv;
        navswitch.data = mj["is_auto"].get<bool>();

        (mj["is_auto"].get<bool>()) ? guicmdRecv.nav_mode = 0 : guicmdRecv.nav_mode = 1;
        if (!mj["goal_lat"].is_null() && !mj["goal_lon"].is_null())
        {
            std::string lat_str = mj["goal_lat"].get<std::string>();
            std::string lon_str = mj["goal_lon"].get<std::string>();

            if (lat_str != "nan" && lon_str != "nan")
            {
                guicmdRecv.goal_lat = std::stof(lat_str);
                guicmdRecv.goal_lon = std::stof(lon_str);
            }
        }
        guicmdRecv.set_search_skew = mj.value("to_skew", -1);
        guicmdRecv.target_cone_id = mj.value("colourId", -1);
        guicmdRecv.search_skew = mj.value("skew", -1);

        guicmds_pub->publish(guicmdRecv);
        navSwitchCmd_pub->publish(navswitch);
    }

private:
    rclcpp::Subscription<custom_msgs::msg::MarkerTag>::SharedPtr marker;
    rclcpp::Subscription<custom_msgs::msg::GpsDetails>::SharedPtr gps;
    rclcpp::Subscription<custom_msgs::msg::ImuData>::SharedPtr imu;
    rclcpp::Subscription<custom_msgs::msg::PlannerStatus>::SharedPtr plan;

    rclcpp::Publisher<custom_msgs::msg::GuiCommand>::SharedPtr guicmds_pub;
    rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr navSwitchCmd_pub;

    rclcpp::TimerBase::SharedPtr timer_;

private:
    std::unordered_map<std::string, std::string> data_map;
    int sock;
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<gui_comm>());
    rclcpp::shutdown();
    return 0;
}