# srs_ingest_helper


SRS实现了拉流功能，但是此功能需要修改配置文件，且不提供API，如果需要动态添加或删除拉流任务，自己修改配置文件是比较麻烦的。

SRS内部是通过ffmpeg实现拉流的，就是启动一个ffmpeg进程，把流拉过来，再推给SRS，其实这个工作我们自己也可以做，而且做的更加自动化。

这就是此程序所做的工作了。对外提供http json api, 控制 ffmpeg实现拉流并转推给SRS。

ffmpeg和SRS程序要放到bin目录下，程序启动后会自动启动SRS，并读取tasks.csv中的任务，启动ffmpeg进程进行拉流。

SRS的编译参见SRS项目主页 https://github.com/ossrs/srs

ffmpeg的编译参见https://github.com/markus-perl/ffmpeg-build-script



json API参见 api.txt

如果不想编译只要运行程序可以点击release
