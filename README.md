## EdgePreserving-Blur
This program creates beautiful blurred images while preserving the edges, texture and details in them. <br />
Basically, it's a c++ implementation of "**Edge-preserving multiscale image decomposition based on local extrema**" by Subr, Kartic, Cyril Soler, and Frédo Durand. ACM Transactions on Graphics (TOG) 28.5 (2009): 147.<br />

| Input | k=5 | k=13 |
| :----: | :----: | :----: |
| ![alt text](https://github.com/yafangshih/EdgePreserving-Blur/blob/master/data/input/glassblue.jpg) <br /> [Chihuly Garden and Glass](https://www.chihulygardenandglass.com/) | ![alt text](https://github.com/yafangshih/EdgePreserving-Blur/blob/master/data/output/glassblue/glassblue-k5.jpg) | ![alt text](https://github.com/yafangshih/EdgePreserving-Blur/blob/master/data/output/glassblue/glassblue-k13.jpg) |
| ![alt text](https://github.com/yafangshih/EdgePreserving-Blur/blob/master/data/input/glassred.jpg) <br /> [Chihuly Garden and Glass](https://www.chihulygardenandglass.com/) | ![alt text](https://github.com/yafangshih/EdgePreserving-Blur/blob/master/data/output/glassred/glassred-k5.jpg) | ![alt text](https://github.com/yafangshih/EdgePreserving-Blur/blob/master/data/output/glassred/glassred-k13.jpg) |
| ![alt text](https://github.com/yafangshih/EdgePreserving-Blur/blob/master/data/input/taipei101.jpg) [Dean Cheng](http://crazydean0420.phootime.com/) | ![alt text](https://github.com/yafangshih/EdgePreserving-Blur/blob/master/data/output/taipei101/taipei101-k5.jpg) | ![alt text](https://github.com/yafangshih/EdgePreserving-Blur/blob/master/data/output/taipei101/taipei101-k13.jpg) |

### Dependencies <br />
 * OpenCV <br />
 * Eigen <br />

### Building <br />
```Shell
$ git clone https://github.com/yafangshih/EdgePreserving-Blur.git
$ cd EdgePreserving-Blur
$ mkdir build
$ cd build
$ cmake ..
$ make
```

### Example <br />
```
$ cd EdgePreserving-Blur
$ bulid/EdgePreservingBlur data/input/taipei101.jpg 3
```
