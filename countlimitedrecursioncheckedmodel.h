#ifndef COUNTLIMITEDRECURSIONCHECKEDMODEL_H
#define COUNTLIMITEDRECURSIONCHECKEDMODEL_H

#include <QStandardItemModel>

struct LevelState
{
	int level,max,checked;
	LevelState(int level,int max,int checked):level(level),max(max),checked(checked)
	{

	}
};

class CountLimitedRecursionCheckedModel : public QStandardItemModel
{
	Q_OBJECT
public:
	explicit CountLimitedRecursionCheckedModel(QObject *parent=nullptr);
	bool setLevelLimit(int level,int max);
	void removeLevelLimit(int level);
	int getDepth(QStandardItem *item);
	int getCheckedCount(const int depth);
private slots:
	void slotItemRecursionCheck(QStandardItem *item);
	void slotEmitSignalItemChanged(QStandardItem *item);
private:
	std::vector<LevelState> levelLimitVector;
	std::map<int,int> checked;
	size_t getLevelLimitIndex(int level);
	void calculateChildrenIncrement(QStandardItem *item,Qt::CheckState checkState);
	void calculateSelfIncrement(QStandardItem *item);
	void calculateParentIncrement(QStandardItem *parent,QStandardItem *item,Qt::CheckState checkState);
	void checkChildren(QStandardItem *item,Qt::CheckState checkState);
	void checkParent(QStandardItem *item);
	void recursionCheck(QStandardItem *item);
	bool setData(const QModelIndex &index,const QVariant &value,int role=Qt::EditRole) override;
	void traverseTree(const int depth,QStandardItem *qsi,int &checkedCount);
signals:
	void signalLevelLimitExceeded(std::vector<LevelState> levelVector);
	//代替itemChanged
	void signalItemChanged(QStandardItem *item);
};

#endif // COUNTLIMITEDRECURSIONCHECKEDMODEL_H
