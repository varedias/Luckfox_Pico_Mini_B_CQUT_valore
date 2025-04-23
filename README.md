valore 基于luckfox pico mini b的图片与激光数据处理的可联网系统
=
硬件设备需求
-
1.Luckfox Pico Mini B 
2.Sc3336 摄像模块
3.MaixSense A010 激光雷达模块
4.RTL8723BS wifi模块
5.搭载有Ubuntu系统的计算机

代码模块整体概述
-
*MainCode*  用于存储需要存放在Luckfox开发板里面的代码和配置文件
*Recieve_Picture* 用于存放可在windows系统运行的用于测试拍摄模块和代码的文件
*Recieve_ridar_2*  用于存放可在windows系统下运行的用于测试激光雷达模块的代码
**注意** 在windows上运行的代码的IP要根据自己开发板分配到的IP进行更改

SDK下载途径
-
*https://blog.csdn.net/qq_28877125/article/details/133148418*  
这个SDK是用于适配RTL8723BS模块的
由于使用激光雷达使用到了串口所以还需要在这个SDK的基础上进行进一步的修改，修改部分主要是设备树里面的串口部分
具体修改方式可以参考Luckfox官网文件

供电注意事项
-
由于MaxiSense A010默认是不开启串口通讯的所以需要进行AT指令初始化，具体请参考
*https://wiki.sipeed.com/hardware/zh/maixsense/maixsense-a010/at_command.html*
需要注意的是串口的输出波特率需要为115200并且一定需要使用开发板的引脚或者电脑给激光雷达供电，只有这样才可以初始化成功
这个原因目前我还没有探究明白，如果只是单单使用一个5V电源给激光雷达供电就没有办法实现初始化！ 
