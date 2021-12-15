# CountLimitedRecursionCheckedTreeModel
用于QTreeView的层级选择限制model，可为每层设置最大可选数目  
## 用法
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
