# 模型准备

## Without AIPP
### SSD 300
```
omg --model=vgg_ssd_300.prototxt --weight=vgg_ssd_300.caffemodel --framework=0 --output=./ssd_300
```

### SSD 512
```
omg --model=vgg_ssd_512.prototxt --weight=vgg_ssd_512.caffemodel --framework=0 --output=./ssd_512
```

在这用转换方式下，转换后的模型输入和原模型一致。

## With AIPP
##加上绝对路径

```
omg --model=vgg_ssd_512.prototxt --weight=vgg_ssd_512.caffemodel --framework=0 --insert_op_conf=aipp.cfg --output=./ssd_512

```
##aipp

根据Atlas500模型转换指导的AIPP配置 
将YUV420SP_U8 转 BGR888_U8
