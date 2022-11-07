#include "countlimitedrecursioncheckedmodel.h"
#include <QDebug>

CountLimitedRecursionCheckedModel::CountLimitedRecursionCheckedModel(QObject *parent) : QStandardItemModel(parent)
{
	QStandardItem *item=new QStandardItem("全选");
	item->setCheckable(true);
	item->setAutoTristate(true);
	appendRow(item);
	connect(this,&CountLimitedRecursionCheckedModel::itemChanged,this,&CountLimitedRecursionCheckedModel::slotItemRecursionCheck);
}

void CountLimitedRecursionCheckedModel::checkChildren(QStandardItem *item,Qt::CheckState checkState)
{
	if(item->hasChildren())
		for(int i=0;i<item->rowCount();checkChildren(item->child(i++),checkState));
	//防止重复勾选
	if(item->checkState()!=checkState)
		item->setCheckState(checkState);
}

void CountLimitedRecursionCheckedModel::checkParent(QStandardItem *item)
{
	QStandardItem *parent=item->parent();
	if(parent==nullptr)
		return;
	int i=0,s=parent->rowCount();
	Qt::CheckState checkState=item->checkState();
	for(;i<s;i++)
		if(parent->child(i)->checkState()!=checkState)
		{
			if(parent->checkState()!=Qt::CheckState::PartiallyChecked)
				parent->setCheckState(Qt::CheckState::PartiallyChecked);
			break;
		}
	if(i==s)
		//同级标志都和自己一致，则修改父级为相同标志
		parent->setCheckState(checkState);
	//修改父级标志后再向上递归
	checkParent(parent);
}

void CountLimitedRecursionCheckedModel::slotItemRecursionCheck(QStandardItem* item)
{
	disconnect(this,&CountLimitedRecursionCheckedModel::itemChanged,this,&CountLimitedRecursionCheckedModel::slotItemRecursionCheck);
	size_t s=levelLimitVector.size();
	//保存点击前状态
	std::vector<LevelState> tempLevelLimitVector=levelLimitVector;
	Qt::CheckState checkState=item->checkState();
	if(s>0)
	{
		disconnect(this,&CountLimitedRecursionCheckedModel::itemChanged,this,&CountLimitedRecursionCheckedModel::slotItemRecursionCheck);
		//对所有层进行计数
		calculateChildrenIncrement(item,checkState);
		calculateSelfIncrement(item);
		calculateParentIncrement(item->parent(),item,checkState);
		//只返回超出限制的层的vector
		std::vector<LevelState> levelVector;
		for(size_t i=0;i<s;i++)
			//和计算后的状态vector比较
			if(levelLimitVector[i].checked>levelLimitVector[i].max)
				//返回变量里存计算前vector状态
				levelVector.push_back(tempLevelLimitVector[i]);
		if(levelVector.size()>0)
		{
			//有某层超出选择限制，则回到勾选前状态，并跳过后续的递归勾选
			item->setCheckState(item->data().value<Qt::CheckState>());
			//还原所有层的计数状态
			levelLimitVector=tempLevelLimitVector;
			//返回超出选择限制的层的点击前状态
			emit signalLevelLimitExceeded(levelVector);
		}
		else
			recursionCheck(item);
	}
	else
		//s==0说明不存在限制条件
		recursionCheck(item);
	connect(this,&CountLimitedRecursionCheckedModel::itemChanged,this,&CountLimitedRecursionCheckedModel::slotItemRecursionCheck);
}

bool CountLimitedRecursionCheckedModel::setLevelLimit(int level,int max)
{
	//set前检查该层已选择数量是否超限，超出返回false
	int checkedCount=getCheckedCount(level);
	if(checkedCount>max)
		return false;
	//返回true代表设置限制成功，返回false代表该层当前已勾选的item数量已经超出了max，设置失败
	size_t index=getLevelLimitIndex(level);
	if(index<levelLimitVector.size())
		levelLimitVector[index].max=max;
	else
		//获取该层已选择的项数量
		levelLimitVector.push_back(LevelState(level,max,checkedCount));
	return true;
}

void CountLimitedRecursionCheckedModel::removeLevelLimit(int level)
{
	size_t index=getLevelLimitIndex(level);
	if(index<levelLimitVector.size())
		levelLimitVector.erase(levelLimitVector.begin()+index);
}

void CountLimitedRecursionCheckedModel::calculateChildrenIncrement(QStandardItem *item,Qt::CheckState checkState)
{
	//向下递归计数，不包括自己这层
	//先递归到最底层再计数
	if(item->hasChildren())
		for(int i=0;i<item->rowCount();calculateChildrenIncrement(item->child(i++),checkState));
	int level=getDepth(item);
	size_t index=getLevelLimitIndex(level);
	//vector中无此层的限制信息或当前递归到的被点击的item子项的标志未改变，则该子项不参与子项变更计数
	if(index<levelLimitVector.size()&&checkState!=item->checkState())
		switch(checkState)
		{
		case Qt::CheckState::Checked:
			levelLimitVector[index].checked++;
			break;
		case Qt::CheckState::Unchecked:
			levelLimitVector[index].checked--;
			break;
		default:;
		}
}

void CountLimitedRecursionCheckedModel::calculateParentIncrement(QStandardItem *parent,QStandardItem *item,Qt::CheckState checkState)
{
	//向上递归计数，不包含自己这层
	if(parent==nullptr)
		return;
	//初次调用时checkState为item的实际点击状态，因为该次点击是由鼠标实际更改，之后向上递归时均为item应为的假定选择状态
	int i=0,s=parent->rowCount();
	//判断parent除item以外的children的点击状态是否跟item的假定选择状态相同
	for(;i<s;i++)
	{
		QStandardItem *it=parent->child(i);
		if(it!=item&&checkState!=it->checkState())
			break;
	}
	//i==s代表parent的其他child的选择状态全等于item的假定选择状态，此时parent的选择状态应假定与item的假定选择状态相同
	//	若parent当前状态为未选择，item假定状态非未选择，则parent所属层+1
	//	若parent当前状态非未选择，item假定状态为未选择，则parent所属层-1
	if(i==s)
	{
		if(parent->checkState()==Qt::CheckState::Unchecked)
		{
			if(checkState!=Qt::CheckState::Unchecked)
				levelLimitVector[getLevelLimitIndex(getDepth(parent))].checked++;
		}
		else if(checkState==Qt::CheckState::Unchecked)
			levelLimitVector[getLevelLimitIndex(getDepth(parent))].checked--;
	}
	//i<s代表parent的其他child的选择状态不全等于item的假定选择状态，此时parent的选择状态应假定为部分选择
	//	若parent当前选择状态为未选择，则parent所属层+1
	else
	{
		checkState=Qt::CheckState::PartiallyChecked;
		if(parent->checkState()==Qt::CheckState::Unchecked)
			levelLimitVector[getLevelLimitIndex(getDepth(parent))].checked++;
	}
	//第三个参数传parent当前选择状态的假定值
	calculateParentIncrement(parent->parent(),parent,checkState);
}

void CountLimitedRecursionCheckedModel::recursionCheck(QStandardItem *item)
{
	connect(this,&CountLimitedRecursionCheckedModel::itemChanged,this,&CountLimitedRecursionCheckedModel::slotEmitSignalItemChanged);
	checkChildren(item,item->checkState());
	//当前item被点击时因参与计算没有发出信号，这里要补发一个
	emit itemChanged(item);
	checkParent(item);
	disconnect(this,&CountLimitedRecursionCheckedModel::itemChanged,this,&CountLimitedRecursionCheckedModel::slotEmitSignalItemChanged);
}

unsigned CountLimitedRecursionCheckedModel::getLevelLimitIndex(int level)
{
	//根据层级获取层级限制状态信息在vector的位置，若不存在则返回levelLimitVector.size()
	size_t i=0,s=levelLimitVector.size();
	for(;i<s;i++)
		if(levelLimitVector[i].level==level)
			return i;
	return s;
}

bool CountLimitedRecursionCheckedModel::setData(const QModelIndex &index,const QVariant &value,int role)
{
	//重写Qt::CheckStateRole的setData事件，在勾选后把勾选前的勾选状态存到Qt::UserRole+1
	if(role==Qt::CheckStateRole)
	{
		disconnect(this,&CountLimitedRecursionCheckedModel::itemChanged,this,&CountLimitedRecursionCheckedModel::slotItemRecursionCheck);
		//data()的默认role为UserRole+1
		//保存点击的item的checkState,用于判断是否要增加该item所属层的checkCount，以及超出max后撤销更改
		QStandardItemModel::setData(index,itemFromIndex(index)->checkState(),Qt::UserRole+1);
		connect(this,&CountLimitedRecursionCheckedModel::itemChanged,this,&CountLimitedRecursionCheckedModel::slotItemRecursionCheck);
	}
	return QStandardItemModel::setData(index,value,role);
}

void CountLimitedRecursionCheckedModel::calculateSelfIncrement(QStandardItem *item)
{
	//判断被点击的item的checkedCount加还是减
	if(item->data()==Qt::CheckState::Unchecked)
	{
		if(item->checkState()!=Qt::CheckState::Unchecked)
			levelLimitVector[getLevelLimitIndex(getDepth(item))].checked++;
		return;
	}
	if(item->checkState()==Qt::CheckState::Unchecked)
		levelLimitVector[getLevelLimitIndex(getDepth(item))].checked--;
}

void CountLimitedRecursionCheckedModel::slotEmitSignalItemChanged(QStandardItem *item)
{
	//在计数完之后对外发射item改变信号
	emit signalItemChanged(item);
}

int CountLimitedRecursionCheckedModel::getDepth(QStandardItem *item)
{
	QStandardItem *par=item->parent();
	return par==nullptr?0:getDepth(par)+1;
}

int CountLimitedRecursionCheckedModel::getCheckedCount(const int depth)
{
	int checkedCount=0;
	traverseTree(depth,item(0),checkedCount);
	return checkedCount;
}

void CountLimitedRecursionCheckedModel::traverseTree(const int depth,QStandardItem *qsi,int &checkedCount)
{
	int qsiDepth=getDepth(qsi);
	//达到指定深度后不再继续向下搜索
	if(depth==qsiDepth)
	{
		if(qsi->checkState()!=Qt::Unchecked)
			checkedCount++;
	}
	else if(qsiDepth<depth&&qsi->hasChildren())
		for(int i=0;i<qsi->rowCount();traverseTree(depth,qsi->child(i++),checkedCount));
}
