# 基于Qt+FFmpeg的视频监控系统

## 需求分析
假设一个业务场景：每个员工工位旁有两个网络摄像头。老板需要一个员工监控软件，在上班时软件可以拉取RTSP视频流，也可以随时录制视频。这样老板就可以知道谁在摸鱼了 ◕‿◕  
为防有人上纲上线，在此特别声明：上面的场景是开玩笑的。监控软件有很多使用场景：学校，公安，交通，考试，零售等等。

## 功能
* 支持画面按人数分页（8/12/24/28），默认24人画面  
* 支持单人切换大屏（鼠标右键）  
* 可同时播放和录制48路视频  

## UI
单页24人
![](./screenshot/48smallScreen.png)
3页，每页8人
![](./screenshot/16smallScreen.png)
单人大屏
![](./screenshot/bigScreen.png)

## 使用
目前配置文件`BetaVideoMonitorClient.ini`可配置两路地址：leftUrl和rightUrl，24人共用这两个地址。  
leftUrl=rtsp://user:password@192.168.1.20:554/Streaming/Channels/101
rightUrl=rtsp://user:password@192.168.1.20:554/Streaming/Channels/101

如果你没有网络摄像头，可以用在线HTTP地址试用，例如：
https://sf1-hscdn-tos.pstatp.com/obj/media-fe/xgplayer_doc_video/mp4/xgplayer-demo-720p.mp4

## 技术实现
* 界面实现用Qt  
* 拉流解码播放用FFmpeg  
* 大屏播放和视频存储用IPC主码流，小屏播放用IPC子码流  
* 目前已支持d3d9渲染yuv，但是测试多路渲染发现效果不好

## 环境依赖
### windows
VS：推荐VS2019以后的版本  
Qt：推荐Qt5.12以后的版本  
  
解决方案仅支持64位的Debug/Release，32位可自行适配

## TODO
- [ ] GPU解码和渲染

