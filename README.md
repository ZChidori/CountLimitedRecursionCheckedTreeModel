# CountLimitedRecursionCheckedTreeModel
用于QTreeView的层级选择限制model，可随时为每层设置最大可选数目  
## 用法
public函数：
```c++
//设置level层的最大可选数量，若当前选择数量已超出设置值则返回false
bool setLevelLimit(int level,int max);
//若已设置level层选择限制则移除
void removeLevelLimit(int level);
//获取指定item深度
int getDepth(QStandardItem *item);
//获取depth层当前已选数量
int getCheckedCount(const int depth);
```
信号：
```c++
//勾选后超出该层限制则发送该信号，levelVector为当前设置过层的限制信息
void signalLevelLimitExceeded(std::vector<LevelState> levelVector);
//代替原生itemChanged信号
void signalItemChanged(QStandardItem *item);
```
