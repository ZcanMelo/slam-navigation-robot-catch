import socket

#可以手动输入本机ip地址，若有多个网口，服务器想从那个网口接收数据，就输入那个网口的ip
#hostname = socket.gethostname()  #可以用 .gethostname()函数来自动得到主机ip，不用手动输入了
#host = socket.gethostbyname(hostname)
host = '192.168.2.107'    #客户端连接到服务器的ip
port = 5270  #端口
sever_address = (host, port)     #创建元组作为 socket.bind()函数的输入，

text_sever = socket.socket(socket.AF_INET, socket.SOCK_STREAM)     #创建一个socket对象为text_sever 为服务器
text_sever.bind(sever_address)    #.bind() 函数绑定端口，该服务器监听此端口
text_sever.listen(4)         #开启监听，同时接入数量最多为4

succeed_flag = 'sok'
while True :
    try:
        print(host)
        print('waiting connect')
        text_client_socket, text_client_address = text_sever.accept()              #accept() 函数，堵塞等待client的连接，连接到后才会执行下一条语句
        print(text_client_address[0] + 'is connected!')

        while True :
            receive_text = text_client_socket.recv(1024)  .decode()            #接收client发送的数据，数据最大为1024 ；此处可以看出接收用户数据测试
            print(receive_text)
            text_client_socket.send(succeed_flag.encode())                     #发送给client ok ，反馈自己确实接收到数据

    finally:
        print('work over!')
————————————————
版权声明：本文为CSDN博主「月早十」的原创文章，遵循CC 4.0 BY-SA版权协议，转载请附上原文出处链接及本声明。
原文链接：https://blog.csdn.net/qq_41224270/article/details/127916407
