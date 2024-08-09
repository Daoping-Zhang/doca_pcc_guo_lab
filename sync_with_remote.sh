#!/bin/bash

# 本地和远程目录
LOCAL_DIR="/home/daoping_223/dpu_work_space"
REMOTE_USER="zdp"
REMOTE_HOST="192.168.100.2"
REMOTE_DIR="/opt/mellanox/doca/applications/pcc"

# 初次同步
rsync -avz --delete $LOCAL_DIR/ $REMOTE_USER@$REMOTE_HOST:$REMOTE_DIR
rsync -avz --delete $REMOTE_USER@$REMOTE_HOST:$REMOTE_DIR/ $LOCAL_DIR

# 启动 inotifywait 监控本地目录的变化
inotifywait -m -r -e create,modify,delete,move $LOCAL_DIR | while read path action file; do
    rsync -avz --delete $LOCAL_DIR/ $REMOTE_USER@$REMOTE_HOST:$REMOTE_DIR
done &
LOCAL_WATCH_PID=$!

# 启动 inotifywait 监控远程目录的变化
ssh $REMOTE_USER@$REMOTE_HOST "inotifywait -m -r -e create,modify,delete,move $REMOTE_DIR" | while read path action file; do
    rsync -avz --delete $REMOTE_USER@$REMOTE_HOST:$REMOTE_DIR/ $LOCAL_DIR
done &
REMOTE_WATCH_PID=$!

# 等待
wait $LOCAL_WATCH_PID $REMOTE_WATCH_PID

