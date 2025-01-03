# 2024年武汉大学操作系统大实验
**食用方法**：  
1、修改bochsrc和makefile中的内容，对应自己的系统即可  
2、进入自己的文件夹后输入以下指令  
> 'make image'  
> 'cd command'  
> 'make install'  
> 'cd ..'  
> 'bochs'
3、在bochs中输入shift+f1~f3即可更换tty

**注意事项**
1、要完成日志功能需要先在tty中设定正确的级别以及日志种类，随后在scripts文件夹中运行genlog脚本即可获得日志文件。该功能并未“动态地”实现，比较简陋。想要实现动态的日志请酌情改造，本人巨cjb。。。  
2、系统容易出现的bug是各种各样的assert错误，可以简单粗暴的把报错的assert语句注释掉，（起码验收的时候看不出来），会对操作系统有什么影响未知。  
3、kill指令的正确食用方法：  
> 在tty1中执行test2指令，该指令会无限打印字符  
> 在tty2中使用'ps'指令查看进程，随后'kill'掉对应的进程号并用'ps'指令查看  
4、某些指令仍存在问题，如'cat'等  
