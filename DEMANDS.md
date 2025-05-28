## 技术规范
- 通信协议：Socket 
- 加密方案：ElGamal密钥交换 + SM4-CBC加密
- 数据格式：JSON
- 连接模式：持久化双工连接

## 架构
使用对等节点，既都在一个core类中实现，但有两种启动模式
- Server：主动启动并等待连接
- Client：连接到指定server节点

## 工作流程

### 阶段1：WebSocket连接建立
1. Server启动WebSocket服务监听指定端口
2. Client建立WebSocket连接到Server
3. 握手完成后建立持久化双工通道

### 阶段2：密钥交换
1. 双方通过WebSocket消息交换ElGamal公钥参数(p,g,y)
2. 调用ReceiveSecret和SendSecret交换得到key1和key2
3. 在日志输出key1和key2以便于调试

### 阶段3：实时加密通信
1. 用户输入消息后立即加密发送
2. 对方实时接收、解密并显示
3. 支持真正的双向实时对话
4. 这部分通过DecryptMessage和EncryptMessage实现

## 技术要求
- 使用现有的MessageEncryptor类处理加密解密，具体使用参照/encrypter/test_encrypter
- 通信可以使用任何socket库哪个好用就用那个
- 不可修改除core目录外的任何代码
- 可以重构原有的额core代码，可以任意的删改
- 编写测试文件，分别测试client模式和server模式