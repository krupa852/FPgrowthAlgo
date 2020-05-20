#include <unordered_map>
#include <set>
#include <list>
#include <algorithm>
#include<iostream>
#include <cmath>
#include <vector>
#include <fstream>
#include <sstream>
#include<map>

using namespace std;


struct FrequentItemsets
{
        vector<vector<string>> itemsets;
        vector<int> supports;
};

struct FPTreeNode
{
    	string item;
	int supcount;
	vector<FPTreeNode *> child; 
    	FPTreeNode *parent; 
   
};

vector<pair<string,vector<FPTreeNode* > > > Table;

vector<pair<vector<string>,int> > frequentList;

fstream in;
float minSup;


void LoadFile(string filename)
{       
        in.open(filename, fstream::in);
        
        if (!in) {
                cout << "Fail to load file." << endl;
                exit(1);
        }
}


/* Read one transaction at a time */
bool  read_transactions(string filename,vector<string>&transaction)
{       
        string  line;
        transaction.clear();
        size_t start, end;
        while (true)
        {       
                if (!getline(in, line))
                        return false;
                start = line.find_first_of('[');
                end = line.find_first_of(']');
                if (start == line.npos || end == line.npos)
                        continue;
                break;
        }
        istringstream iss(line.substr(start + 1, end - start - 1));
        string it;
        while (getline(iss, it, ','))
                transaction.push_back(it);
        return true;
}

void Reset()
{       
        in.clear();
        in.seekg(0);
}

/*Function to generate frequent one itemset */
FrequentItemsets Frequent_one_itemsets(string filename,vector<string> transaction ,int minSup)
{       
        unordered_map<string, int> map;
        Reset();
        while(read_transactions(filename,transaction))
        {       
                for (auto &item : transaction)
                {       
                        map[item]++;
                }
        }
        
        FrequentItemsets result;
        for (auto &pair : map)
                if (pair.second >= minSup)
                {       
                        result.itemsets.push_back(vector<string>(1, pair.first));
                        result.supports.push_back(pair.second);
                }
        return result;
}

bool cmp_by_value(const pair<string,int>& lhs, const pair<string,int>& rhs) {
    return lhs.second > rhs.second;
}

/* Function to arrange frequent one itemsets based on priority(supportcount in decreasing order) */
void Priority_Order_List(FrequentItemsets output,vector<pair<string,bool> > &priorityList )
{
	map<string,int> supMap;
	for (size_t i = 0; i < output.itemsets.size(); ++i)
	{
        supMap.insert(make_pair(output.itemsets[i][0],output.supports[i])); 
	}
	
	vector<pair<string, int>> supportlist(supMap.begin(), supMap.end());
    	sort(supportlist.begin(), supportlist.end(), cmp_by_value);
	for (int i = 0; i != supportlist.size(); i++) {
		priorityList.push_back(make_pair(supportlist[i].first, false));}
     

}

/*Create Header Table that records the location of all item node. Each time a new node is added to the tree, it needs to be linked to the last node with the same item*/
void Create_Header_Table(vector<pair<string,vector<FPTreeNode* > > > &Table,vector<pair<string,bool> > priorityList)
{
	vector<FPTreeNode *> Nodeptr;
    	Nodeptr.clear();
    	for(int i = 0; i < priorityList.size(); i++)
        Table.push_back(make_pair(priorityList[i].first, Nodeptr));
	
}	

/*Construct FP-Tree by adding new item nodes. If item node exist,increment support count */
void Add_Node_FPTree(vector<pair<string,int>> Treeinsertitems,int Index,FPTreeNode *ptr,vector<FPTreeNode *>& FPTree,vector<pair<string,vector<FPTreeNode* > > > &Table)
{
	bool Current_state = false;
	if(FPTree.size() > 0)
	{
        	for(int i = 0; i < FPTree.size(); i++)
		{
            		if(Treeinsertitems.size() > 0)
                	{
				if(FPTree[i]->item == Treeinsertitems[Index].first)
				{
                    			Current_state = true;
                    			FPTree[i]->supcount += Treeinsertitems[Index].second;
                    			if(Index + 1 < Treeinsertitems.size())
					Add_Node_FPTree(Treeinsertitems,Index+1, FPTree[i], FPTree[i]->child,Table);
                        	
                		}
         		}
        	}
    	}
	if(Current_state==false)
	{	
		if(Treeinsertitems.size() != 0)
		{
			FPTreeNode *Node;
            		Node = new FPTreeNode;
            		Node->item = Treeinsertitems[Index].first;
            		Node->supcount = Treeinsertitems[Index].second;
            		Node->parent = ptr;
			for(int i = 0; i <Table.size(); i++)
			{
                		if(Table[i].first == Treeinsertitems[Index].first)
				{
                    			Table[i].second.push_back(Node);
					
                		}
            		}
			if(Index +1 < Treeinsertitems.size())
			{
                		Add_Node_FPTree(Treeinsertitems,Index+1,Node,Node->child,Table);
            		}
            		FPTree.push_back(Node);
		}
	}
}

/*Read each transaction, order items in the transaction according to priority order. pass the ordered transaction for Add_Node_FPTree function for FPtreeConstruction */
void Insert_Ordered_Trans_FPTree(string filename,vector<string> transaction,vector<pair<string,bool> > priorityList, vector<FPTreeNode *> Root,vector<pair<string,vector<FPTreeNode* > > > &Table)
{
	
	Reset();
	vector<pair<string,int>> Treeinsertitems;
        Treeinsertitems.clear();
        while(read_transactions(filename,transaction))
        {
        	for(int j = 0; j< priorityList.size(); j++)
		{
            		for (int i = 0; i < transaction.size(); i++)
                	{		
                			if (priorityList[j].first == transaction[i])
					{
                    				priorityList[j].second = true;
                    				Treeinsertitems.push_back(make_pair(priorityList[j].first,1) );
					}
                	}
			
            	}
		Add_Node_FPTree(Treeinsertitems,0,NULL,Root,Table);
        	Treeinsertitems.clear();
        	for (int i = 0; i < priorityList.size(); i++)
            		priorityList[i].second = false;
        } 
}
		  
/*traverse each node vertically to get prefix of a given itemnode & store result in prefix pattern.*/
void prefixpath(FPTreeNode *node, int count, int Index, vector<vector<pair<string,int> > > &prefixpattern)
{
    	prefixpattern[Index].push_back(make_pair(node->item, count));
	if(node->parent != NULL)
        	prefixpath(node->parent,count,Index, prefixpattern);
}

/*Construct Conditional FP-tree for each subproblem. */
void Conditional_FPTree(vector<vector<pair<string,int> > > transactions, vector<pair<string,bool> > &priorityList, vector<FPTreeNode *> &Root, vector<pair<string,vector<FPTreeNode* > > > &Table)
{
        vector<pair<string,int>> insertitems;
    	insertitems.clear();
    	for (int i = 0; i < transactions.size(); i++)
    	{
        	for(int j = 0; j< priorityList.size(); j++)
		{
            		for (int k = 0; k < transactions[i].size(); k++)
			{
                		if (priorityList[j].first == transactions[i][k].first)
				{
                    			priorityList[j].second = true;
                    			insertitems.push_back(make_pair(priorityList[j].first, transactions[i][k].second) );
                		}
            		}
        	}
        
        	Add_Node_FPTree(insertitems,0,NULL,Root,Table);
        	insertitems.clear();
        	for (int i = 0; i < priorityList.size(); i ++)
		         priorityList[i].second = false;
	}
}

/*Use the pointer to nodes in the header table to deconstruct the FP-Tree into multiple subtrees,each represent a subproblem.
For each subproblem, traverse the corresponding subtree bottom-up to obtain  prefix paths for the subproblem recursively.Extract the frequent patterns generated for each subproblem*/

void Extract_Frequent_Pattern(vector<pair<string,vector<FPTreeNode* > > > &Table,vector<pair<string,bool> > priorityList,vector<vector<pair<string,int> > > &prefixpattern,vector<string> strvec)
{
	for(vector<pair<string,vector<FPTreeNode* > > >::reverse_iterator it = Table.rbegin(); it != Table.rend(); ++it)
	{
		vector<vector<pair<string,int> > > transactions;
        	map<string,int> frequentTemp;
        	map<string, int> Temp;
        	vector<pair<string,bool> > subpriorityList;
        	vector<FPTreeNode *> subRoot;
        	vector<pair<string,vector<FPTreeNode* > > > subTable;
        	vector<string> strvec1;
        	vector<vector<pair<string,int> > > condpattern(100);
        	int frequentCount=0;
		Temp.clear();
        	prefixpattern.clear();
        	transactions.clear();
        	frequentTemp.clear();
        	subpriorityList.clear();
        	subRoot.clear();
        	subTable.clear();
		strvec1.clear();
		for(int i = 0; i < it->second.size(); i++)
		{
                	frequentCount += it->second[i]->supcount;
			if(it->second[i]->parent != NULL){
               			prefixpath(it->second[i]->parent, it->second[i]->supcount, i, prefixpattern);
            		}
        		
		}	

	 	for(int i = 0; i < strvec.size(); i++){
            		strvec1.push_back(strvec[i]);
        	}
        	strvec1.push_back(it->first);
		frequentList.push_back(make_pair(strvec1, frequentCount));
		for(int i = 0; i < frequentList.size(); i++)
            		sort(frequentList[i].first.begin(), frequentList[i].first.end());
        	for(int i = 0; i < frequentList.size(); i++)
            		sort(frequentList.begin(), frequentList.end());
        	
        	int i = 0;
        	while(i < prefixpattern.capacity())
		{
            		if(prefixpattern[i].size() > 0)
			{
                		for(int j = 0; j < prefixpattern[i].size(); j++)
         			{
                    			if(Temp.count(prefixpattern[i][j].first))
                       				Temp[prefixpattern[i][j].first] += prefixpattern[i][j].second;
                    			else
                        			Temp[prefixpattern[i][j].first] = prefixpattern[i][j].second;
                		}
              
                		transactions.push_back(prefixpattern[i]);
            		}
            		prefixpattern[i].clear();
            		i++;
        	}
        
        	if(Temp.size() > 0)
		{
            		for (map<string,int>::iterator it=Temp.begin(); it!=Temp.end(); ++it)
			{
                		if((float)it->second >= minSup)
                    			frequentTemp[it->first] = it->second;
			}
            		
            
            		vector<pair<string, int>> supportList(frequentTemp.begin(), frequentTemp.end());
            		sort(supportList.begin(), supportList.end(), cmp_by_value);
            
            		for (int k = 0; k != supportList.size(); k++) 
           		subpriorityList.push_back(make_pair(supportList[k].first, false));
            
			Create_Header_Table(subTable,subpriorityList);
            		Conditional_FPTree(transactions, subpriorityList,subRoot,subTable);
            		Extract_Frequent_Pattern(subTable, subpriorityList,condpattern,strvec1);
        	}
	}
}

ostream &operator<<(ostream &os, const vector<string> &itemset)
{
        for (size_t i = 0; i < itemset.size(); ++i)
        {
                os << itemset[i];
                if (i + 1 != itemset.size())
                        os << ", ";
        }
        return os;
}



ostream &operator<<(ostream &os, const FrequentItemsets &result)
{
        size_t size = result.itemsets.size();
        os << "The number of Frequent itemsets: " << size << endl;
        for (size_t i = 0; i < size; ++i)
        os << result.itemsets[i] << ": " << result.supports[i] << endl;
        return os;
}
	
	


int main(int argc, char* argv[])
{
        string db_filename;
	FrequentItemsets Output;
        vector<string> transaction;
	vector<FPTreeNode *> Root;
	vector<pair<string,bool> > priorityList;
	vector<vector<pair<string,int> > > prefixpattern(100);
	vector<string> strvec;
	vector<pair<vector<string>,int> > frequentOutput;
        if(argc!=3)
        {
                printf("Error");
                return 0;
        }
        db_filename = string(argv[1]);
        minSup = (int) ceil(atof(argv[2]));
        LoadFile(db_filename);
	FrequentItemsets output = Frequent_one_itemsets(db_filename,transaction,minSup);
	Priority_Order_List(output,priorityList);
	Create_Header_Table(Table,priorityList);
	Insert_Ordered_Trans_FPTree(db_filename,transaction,priorityList,Root,Table);
	Extract_Frequent_Pattern(Table,priorityList,prefixpattern,strvec);
        for(int i = 1; i < 100; i++){
        	for(int j = 0; j < frequentList.size(); j++){
                	if(frequentList[j].first.size() == i)
			{
                    		Output.itemsets.push_back(frequentList[j].first);
				Output.supports.push_back(frequentList[j].second);	
                	}
            	}
        }
	cout<<Output<<endl;
} 


