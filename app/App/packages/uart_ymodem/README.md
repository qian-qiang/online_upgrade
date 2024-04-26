# uart_ymodem

## 介绍

本软件包是连接uart设备和ymodem的驱动包。

##  支持情况
| 包含设备         |    |
| ---------------- | -------- |
| **通讯接口**     |          |
| uart              |  √       |


##  目录结构

| 名称 | 说明 |
| ---- | ---- |
| demo | 例子目录 |


##  许可证

uart_ymodem package 遵循 Apache-2.0 许可，详见 `LICENSE` 文件。

## 依赖

- RT-Thread 3.0+
- RT-Thread 4.0+

## 设置参数

### 单个包的最大数据发送长度：
`RT_UART_YMODEM_PACK_SIZE_TX`，



## API说明


1. 数据接收反馈

```C
void rt_uart_ymodem_rx(struct rt_uart_ymodem_device *uart_ymodem, const void  *buffer, rt_size_t size)
```
uart给ymodem发送数据，在uart接受解析线程中调用。

|参数|注释|
|----|----|
|uart_ymodem|设备|
|buffer|接收数据指针|
|size| 接收数据长度|
|返回|注释|
|----|----|
|void|没有|

2. 映射发送函数
```C
static int rt_hw_V_ymodem_tx(struct rt_uart_ymodem_device *uart_ymodem,
                                const void       *buffer,
                                rt_size_t         size)
```
ymodem给uart发送数据，里面需要调用uart设备的发送函数，这个函数将会注册到到uart_ymodem设备上。

|参数|注释|
|----|----|
|uart_ymodem|设备|
|buffer|发送数据指针|
|size| 发送数据长度|
|返回|注释|
|----|----|
|int|暂时没有使用，建议正常返回发送长度，不正常返回-RT_ERROR|

3. 注册can_ymodem设备

```C
rt_err_t rt_hw_uart_ymodem_register(struct rt_uart_ymodem_device *uart_ymodem,
                                   const char              *name,
                                   rt_uint32_t              flag,
                                   void                    *data)
```

|参数|注释|
|----|----|
|uart_ymodem|设备|
|name|设备名称|
|flag|设备打开标志|
|data|私有数据|
|返回|注释|
|----|----|
|RT_EOK|正常|
|-RT_ERROR|异常|



## 驱动示例

使用示例在 [driver](./demo) 下。


## 注意事项



## 联系方式 & 感谢

* 维护：qianqiang
* 主页：<https://github.com/qianqiang/rt_uart_ymodem>
* 邮箱：<845343253@qq.com>
