#!/bin/bash

# 初始化环境
source ./devel/setup.bash

# 生成时间戳和日志路径
TIMESTAMP=$(date "+%Y%m%d_%H%M%S")
mkdir -p logs

LOG_OFFLINE="logs/offline_${TIMESTAMP}.log"
LOG_VAD="logs/vad_${TIMESTAMP}.log"
LOG_TEST="logs/test_${TIMESTAMP}.log"

# 带状态反馈的节点启动函数
start_node() {
    local cmd=$1
    local log=$2
    local name=$3
    local delay=$4  # 启动间隔时间
    
    echo -n "正在启动 $name ..."
    $cmd > "$log" 2>&1 &
    local pid=$!
    
    # 等待进程启动
    sleep 1
    if ! ps -p $pid > /dev/null; then
        echo -e "\r[失败] $name 启动失败，请查看 $log"
        exit 1
    fi
    
    # 初始延迟（根据节点类型调整）
    sleep $delay
    echo -e "\r[成功] $name 已启动"
}

# 按顺序启动节点（新增延迟参数）
# 修改后的节点启动命令（添加错误过滤）
start_node "roslaunch handsfree_speech offline_interactive_ros.launch" "$LOG_OFFLINE" "离线交互节点" 5
# 修改VAD节点启动命令，过滤ALSA警告
# 在启动函数中添加环境变量设置
start_node "env ALSA_CONFIG_PATH=/usr/share/alsa/alsa.conf rosrun handsfree_speech vad_record.py 2>/dev/null" "$LOG_VAD" "VAD节点" 3
start_node "rosrun handsfree_speech test.py" "$LOG_TEST" "通信节点" 2


# 清理函数（保持不变）
cleanup() {
    kill -9 $pid_offline $pid_vad $pid_test 2>/dev/null
    echo "已终止所有后台进程"
    exit 0
}

# 注册退出信号捕获（保持不变）
trap cleanup SIGINT SIGTERM

# 优化状态显示界面（新增部分）
echo "======================================"
echo "节点状态："
echo "✔ 离线交互节点 (PID: $pid_offline)"
echo "✔ VAD节点       (PID: $pid_vad)"
echo "✔ 通信节点      (PID: $pid_test)"
echo "======================================"
echo "操作提示："
echo "1. 实时监控节点:watch -n 1 'ps -p $pid_offline,$pid_vad,$pid_test -o pid,comm'"
echo "2. 快速查看日志:tail -n 20 *.log"
echo "======================================"

# 保持脚本运行（保持不变）
while true; do
    sleep 3600
done