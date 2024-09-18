# ch32v307-Whell-Leg

CH32V307_Servo_Test是使用CH307控制SCS15串口总线舵机的测试工程。

CH32V307_Servo_Test工程下的examples文件夹中的每个文件内均包含loop和setup函数，若想使用不同测试指令，则需仅使examples文件夹下的某一文件参与编译。

如：若想使用ping指令测试舵机是否在线，则需仅让examples文件夹下的Ping.c参与编译。

> 注：舵机的默认波特率为1000k，默认id为1。

本工程使用的通信方式为半双工单线UART。默认使用UART2，GPIO口为A2.











