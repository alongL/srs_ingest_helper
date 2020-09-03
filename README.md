# srs_ingest_helper

## 作用
此程序用于控制ffmpeg进程拉流并转推srs，实现自定义拉流。  

## 原理
SRS实现了拉流功能，但是此功能需要修改配置文件，且不提供API，如果需要动态添加或删除拉流任务，自己修改配置文件是比较麻烦的。  
SRS内部是通过ffmpeg实现拉流的，就是启动一个ffmpeg进程，把流拉过来，再推给SRS，其实这个工作我们自己也可以做，而且做的更加自动化。  
这就是此程序所做的工作了。  

## 编译和使用

执行下列命令进行编译，没有其他依赖
```
cd src
make -j4
```


ffmpeg和SRS程序要放到bin目录下，程序启动后会自动启动SRS，并读取tasks.csv中保存的任务，启动ffmpeg进程进行拉流。  

SRS的编译参见SRS项目主页 https://github.com/ossrs/srs  

ffmpeg的编译参见https://github.com/markus-perl/ffmpeg-build-script

## API
程序对外提供http json api,用于添加和删除媒体源。

GET http://localhost:8086/addstream?src=rtmp://abc.com/live/my&dest=/live/my  
GET http://localhost:8086/delstream?dest=/live/my  
GET http://localhost:8086/listmedia  

json API 详情参见 api.txt




## release
如果不想编译只要程序，可以点击 release 下载编译后的程序
（包含ffmpeg和srs,可直接运行于centos 7.6 ubuntu 18.04)


