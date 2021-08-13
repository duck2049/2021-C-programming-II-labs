#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include "windows.h "
#include "shellapi.h " 
using namespace std;

#define N 1000

class line_breaks {
public:
	int cnt;
	int array[N];
};

class md_lines {
public:
	string array[N];//除去\n和标识符的string
	int title[N];//数字表示#的个数
	int dash[N];//数字表示层级，1,2,3，...
	int number[N];//同上
	int star[N];//同上
	int blanks[N];//末尾是否有两个及以上的空格
	int lines_cnt;
};

class paragraph {//markdown中的一段，相当于HTML中的一行
public:
	int start;//起始行
	int end;//结束行
	int title;//数字表示#个数
	int olist;//数字表示层级
	int uolist;//同上
	int blanks;
	string str;//去掉特殊字符的内容部分，并转换为HTML
	paragraph() {
		start = end = title = olist = uolist = blanks = 0;
	}


};

#define NODE_TYPE_ORDERED 1
#define NODE_TYPE_UNORDERED 2
#define NODE_TYPE_COMMON 3
#define NODE_TYPE_SUBLIST 4
struct llnode {
	llnode* next, * pre;
	string str;
	int level;
	int type;//NODE_TYPE_XXXXXX
	llnode(paragraph* input) {
		next = pre = NULL;
		str = input->str;
		if (input->olist != 0) {
			type = NODE_TYPE_ORDERED;
			level = input->olist;
		}
		else if (input->uolist != 0) {
			type = NODE_TYPE_UNORDERED;
			level = input->uolist;
		}
		else {
			type = NODE_TYPE_COMMON;
			level = 0;
		}
	}
	llnode() {
		next = pre = NULL;
		str = "";
		level = 0;
		type = 0;
	}
};

class llist {
public:
	llnode head, tail;
	int count;
	int count_node() {
		llnode* ptr = head.next;
		int count = 0;
		while (ptr != &tail) {
			count++;
			ptr = ptr->next;
		}
		return count;
	}
	llist() {
		head.pre = NULL;
		head.next = &tail;

		tail.pre = &head;
		tail.next = NULL;

		count = 0;
	}
	void add_node(paragraph* input) {
		llnode* newnode = new llnode(input);
		newnode->pre = tail.pre;
		newnode->next = &tail;
		tail.pre->next = newnode;
		tail.pre = newnode;
		count++;
	}
	void merge(llnode* ptr1, llnode* ptr2, int type) {
		string ans;
		llnode* ptr = ptr1;
		while (ptr != ptr2) {
			if (ptr->type == NODE_TYPE_ORDERED || ptr->type == NODE_TYPE_UNORDERED) {
				ans += "<li>" + ptr->str + "</li>\n";
			}
			else if (ptr->type == NODE_TYPE_COMMON) {
				ans += ptr->str + "<br>\n";
			}
			else {
				ans += ptr->str + "\n";
			}
			ptr = ptr->next;
		}
		if (type == NODE_TYPE_ORDERED) {
			ans = "<ol>\n" + ans + "\n</ol>\n";
		}
		else if (type == NODE_TYPE_UNORDERED) {
			ans = "<ul>\n" + ans + "\n</ul>\n";
		}
		else {
			ans = ans + "\n";
		}

		ptr1->next = ptr2;
		ptr = ptr2->pre;
		ptr1->level = ptr1->pre->level;
		ptr1->str = ans;
		ptr1->type = NODE_TYPE_SUBLIST;
		while (ptr != ptr1) {
			llnode* tmp = ptr;
			ptr = ptr->pre;
			delete(tmp);
			count--;
		}
		ptr2->pre = ptr1;
	}
	string solve_list() {
		while (count_node() > 1) {
			// cout <<"debug count:"<< count << endl;

			llnode* p = head.next;
			// while (p != &tail) {
			// 	cout << p->level << ' ' << p->type << ' ' << p->str << endl;
			// 	p = p->next;
			// }
			//do merge

			//get maxlevel
			int maxlevel = 0;
			llnode* ptr = head.next;
			while (ptr != &tail) {
				if (ptr->level > maxlevel) {
					maxlevel = ptr->level;
				}
				ptr = ptr->next;
			}
			//find node with maxlevel

			ptr = head.next;
			while (ptr != &tail) {
				if (ptr->level == maxlevel) {
					llnode* ptr2 = ptr->next;

					int type = ptr->type;
					while ((ptr2->level == maxlevel && (ptr2->type == type || ptr2->type == NODE_TYPE_SUBLIST)) || ptr2->type == NODE_TYPE_COMMON) {
						ptr2 = ptr2->next;
					}
					merge(ptr, ptr2, type);
				}
				ptr = ptr->next;
			}

		}
		return head.next->str;

	}
};



class Trans {
public:
	string md_file;
	void input(string str);
	line_breaks break_ins;

	md_lines md_lines;
	void getRawLines();
	string solve(string str);

	paragraph par[N];
	int par_cnt;
	void getParagraph();
	void getParagraphString();


	string getHTMLLines();
	string title(int i);
	string list(int m, int n);

	void output(string str);
};

void Trans::input(string file) {
	fstream fin(file.c_str(), ios::in);
	break_ins.cnt = 1;
	break_ins.array[0] = -1;//第一个\n是不存在的，从【1】开始存位置
	char c = fin.get();


	//while (c != EOF) {
	//	if (c == '\n') {
	//		break_ins.array[break_ins.cnt] = (int)fin.tellg() - break_ins.cnt-1;//c在文本中的位置，\n占一个字符，这个位置是他的位置
	//		//cout << break_ins.cnt << ':';
	//		//cout << break_ins.array[break_ins.cnt] << endl;
	//		break_ins.cnt++;
	//	}
	//	md_file += c;
	//	c = fin.get();
	//}
	//break_ins.cnt--;


	while (c != EOF) {
		md_file += c;
		if (c == '\n') {
			break_ins.array[break_ins.cnt] = md_file.length() - 1;//c在文本中的位置，\n占一个字符，这个位置是他的位置
			//cout << break_ins.cnt << ':';
			//cout << break_ins.array[break_ins.cnt] << endl;
			break_ins.cnt++;
		}
		c = fin.get();
	}
	md_file += "\n";
	break_ins.array[break_ins.cnt] = md_file.length() - 1;

	fin.close();
	/*for (int i = 0; i <= break_ins.cnt; i++) {
		cout << "i=" << i << ",data=" << break_ins.array[i] << endl;
	}*/
}

void Trans::getRawLines() {

	//初始化
	md_lines.lines_cnt = 0;
	memset(md_lines.dash, 0, sizeof(md_lines.dash));
	memset(md_lines.star, 0, sizeof(md_lines.star));
	memset(md_lines.number, 0, sizeof(md_lines.number));
	memset(md_lines.title, 0, sizeof(md_lines.title));
	memset(md_lines.blanks, 0, sizeof(md_lines.blanks));


	for (int i = 0; i < break_ins.cnt; i++) {//外循环：遍历所有的\n
		//cout << "i=" << i << endl;
		int p = break_ins.array[i] + 1;//上一个\n结束位置
		int q = break_ins.array[i + 1] - 1;//下一个\n开始前
		int t_cnt = 0;//临时记录空格数量


		//跳过开头的所有空格
		while ((p <= q) && (md_file[p] == ' ')) {
			t_cnt++;
			p++;
		}

		//末尾有没有多个空格
		if (md_file[q] == ' ') {
			int cnt = 0;
			int rear = q;
			while ((rear >= p) && (md_file[rear] == ' ')) {
				cnt++;
				rear--;
			}
			if (cnt >= 2) {
				md_lines.blanks[md_lines.lines_cnt] = cnt;
			}
		}

		//处理#
		if (md_file[p] == '#') {
			while (p <= q && md_file[p] == '#') {
				p++;
				md_lines.title[md_lines.lines_cnt]++;
			}
			if (md_file[p] == ' ') {//匹配到标题行，直接把该行后面的内容写进去
				md_lines.array[md_lines.lines_cnt] = md_file.substr(p + 1, q - p);
				md_lines.lines_cnt++;
				//cout << "loop" << i << " " << md_lines.lines_cnt << endl;
				continue;
			}
			else {//匹配失败，清空记录的#个数
				p -= md_lines.title[md_lines.lines_cnt];
				md_lines.title[md_lines.lines_cnt] = 0;
				md_lines.array[md_lines.lines_cnt] = md_file.substr(p, q - p + 1);
				md_lines.lines_cnt++;
			}
			continue;
		}
		//处理-
		if ((p + 1 <= q) && (md_file[p] == '-') && (md_file[p + 1] == ' ') && (t_cnt % 4 == 0)) {
			md_lines.dash[md_lines.lines_cnt] = t_cnt / 4 + 1;
			md_lines.array[md_lines.lines_cnt] = md_file.substr(p + 2, q - p - 1);
			md_lines.lines_cnt++;
			//cout << "loop" << i << " " << md_lines.lines_cnt << endl;
			continue;
		}

		//处理*
		if ((p + 1 <= q) && (md_file[p] == '*') && (md_file[p + 1] == ' ') && (t_cnt % 4 == 0)) {
			md_lines.star[md_lines.lines_cnt] = t_cnt / 4 + 1;
			md_lines.array[md_lines.lines_cnt] = md_file.substr(p + 2, q - p - 1);
			md_lines.lines_cnt++;
			//cout << "loop" << i << " " << md_lines.lines_cnt << endl;
			continue;
		}

		//处理有序列表
		if ((p + 1 <= q) && (p + 2 <= q) && (md_file[p] >= '0') && (md_file[p] <= '9')
			&& (md_file[p + 1] == '.') && (md_file[p + 2] == ' ') && (t_cnt % 4 == 0)) {
			md_lines.number[md_lines.lines_cnt] = t_cnt / 4 + 1;
			md_lines.array[md_lines.lines_cnt] = md_file.substr(p + 3, q - p - 2);
			md_lines.lines_cnt++;
			//cout << "loop" << i << " " << md_lines.lines_cnt << endl;
			continue;
		}




		//普通的行(空行是""，空字符)
		md_lines.array[md_lines.lines_cnt] = md_file.substr(p, q - p + 1);
		md_lines.lines_cnt++;
		//cout << "loop" << i << " " << md_lines.lines_cnt << endl;
	}
}

string Trans::solve(string str) {
	int pos1, pos2, pos3;
	//图片
	if ((pos1 = str.find("![")) != string::npos) {
		string substr1 = str.substr(0, pos1);
		string substr2 = str.substr(pos1 + 2);
		if ((pos2 = substr2.find("](")) != string::npos) {
			pos2 += pos1 + 1;
			string substr22 = str.substr(pos2 + 2);
			if ((pos3 = substr22.find(")")) != string::npos) {//匹配全成功
				pos3 += pos2 + 1;
				substr2 = str.substr(pos1 + 2, pos2 - pos1 - 1);
				string substr3 = str.substr(pos3 + 2);
				string link = str.substr(pos2 + 3, pos3 - pos2 - 2);
				//cout<< solve(substr1) + "<a href=\"" + link + "\">" + solve(substr2) + "</a>" + solve(substr3);
				return solve(substr1 + "<img src=\"" + link + "\" alt=\"" + substr2 + "\">" + substr3);
			}
		}
	}

	//链接
	if ((pos1 = str.find("[")) != string::npos) {
		string substr1 = str.substr(0, pos1);
		string substr2 = str.substr(pos1 + 1);
		if ((pos2 = substr2.find("](")) != string::npos) {
			pos2 += pos1 + 1;
			string substr22 = str.substr(pos2 + 2);
			if ((pos3 = substr22.find(")")) != string::npos) {//匹配全成功
				pos3 += pos2 + 1;
				substr2 = str.substr(pos1 + 1, pos2 - pos1 - 1);
				string substr3 = str.substr(pos3 + 2);
				string link = str.substr(pos2 + 2, pos3 - pos2 - 1);
				return solve(substr1 + "<a href=\"" + link + "\">" + substr2 + "</a>" + substr3);
				//cout << solve(substr1) + "<img src=\"" + link + "\">" + " alt=\"" + solve(substr2) + "\">" + solve(substr3);

			}
		}
	}
	if ((pos1 = str.find("___")) != string::npos) {
		string substr1 = str.substr(0, pos1);
		string substr2 = str.substr(pos1 + 3);
		if ((pos2 = substr2.find("___")) != string::npos) {
			pos2 += pos1 + 3;
			substr2 = str.substr(pos1 + 3, pos2 - pos1 - 3);
			string substr3 = str.substr(pos2 + 3);
			return solve(substr1 + "<b><i>" + substr2 + "</i></b>" + substr3);
		}
	}


	//加粗
	if ((pos1 = str.find("__")) != string::npos) {
		string substr1 = str.substr(0, pos1);
		string substr2 = str.substr(pos1 + 2);
		if ((pos2 = substr2.find("__")) != string::npos) {
			pos2 += pos1 + 2;
			substr2 = str.substr(pos1 + 2, pos2 - pos1 - 2);
			string substr3 = str.substr(pos2 + 2);
			return solve(substr1 + "<b>" + substr2 + "</b>" + substr3);
		}
	}

	//斜体
	if ((pos1 = str.find("_")) != string::npos) {
		string substr1 = str.substr(0, pos1);
		string substr2 = str.substr(pos1 + 1);
		if ((pos2 = substr2.find("_")) != string::npos) {
			pos2 += pos1 + 1;
			substr2 = str.substr(pos1 + 1, pos2 - pos1 - 1);
			string substr3 = str.substr(pos2 + 1);
			return solve(substr1 + "<i>" + substr2 + "</i>" + substr3);
		}
	}
	//加粗&斜体
	if ((pos1 = str.find("***")) != string::npos) {
		string substr1 = str.substr(0, pos1);
		string substr2 = str.substr(pos1 + 3);
		if ((pos2 = substr2.find("***")) != string::npos) {
			pos2 += pos1 + 3;
			substr2 = str.substr(pos1 + 3, pos2 - pos1 - 3);
			string substr3 = str.substr(pos2 + 3);
			return solve(substr1 + "<b><i>" + substr2 + "</i></b>" + substr3);
		}
	}


	//加粗
	if ((pos1 = str.find("**")) != string::npos) {
		string substr1 = str.substr(0, pos1);
		string substr2 = str.substr(pos1 + 2);
		if ((pos2 = substr2.find("**")) != string::npos) {
			pos2 += pos1 + 2;
			substr2 = str.substr(pos1 + 2, pos2 - pos1 - 2);
			string substr3 = str.substr(pos2 + 2);
			return solve(substr1 + "<b>" + substr2 + "</b>" + substr3);
		}
	}

	//斜体
	if ((pos1 = str.find("*")) != string::npos) {
		string substr1 = str.substr(0, pos1);
		string substr2 = str.substr(pos1 + 1);
		if ((pos2 = substr2.find("*")) != string::npos) {
			pos2 += pos1 + 1;
			substr2 = str.substr(pos1 + 1, pos2 - pos1 - 1);
			string substr3 = str.substr(pos2 + 1);
			return solve(substr1 + "<i>" + substr2 + "</i>" + substr3);
		}
	}

	//删除
	if ((pos1 = str.find("~~")) != string::npos) {
		string substr1 = str.substr(0, pos1);
		string substr2 = str.substr(pos1 + 2);
		if ((pos2 = substr2.find("~~")) != string::npos) {
			pos2 += pos1 + 2;
			substr2 = str.substr(pos1 + 2, pos2 - pos1 - 2);
			string substr3 = str.substr(pos2 + 2);
			return solve(substr1 + "<s>" + substr2 + "</s>" + substr3);
		}
	}
	/*string new_str;
	for (int i = 0; i < str.length(); i++) {
		if (str[i] == ' ') {
			while (i < str.length() && str[i] == ' ') {
				i++;
			}
			i--;
			new_str.push_back(' ');
		}
		else if (str[i] == '\n') {
			new_str.push_back(' ');
		}
		else {
			new_str.push_back(str[i]);s
		}
	}
	cout << str << ' ' << new_str << endl;*/
	//cout << str;
	return str;


}

void Trans::getParagraph() {
	int pre = 0;
	par_cnt = 0;

	for (int i = 0; i <= md_lines.lines_cnt; i++) {

		//if (md_lines.title[i] != 0) {//#,该行就是确定的一
		//	if (pre <= i - 1) {
		//		par[par_cnt].start = pre;
		//		par[par_cnt].end = i - 1;
		//		par[par_cnt+1].uolist = (md_lines.dash[i] != 0) ? md_lines.dash[i] : md_lines.star[i];
		//		par_cnt++;
		//	}
		//	par[par_cnt].start = i;
		//	par[par_cnt].end = i;
		//	par[par_cnt].title = md_lines.title[i];
		//	pre = i + 1;
		//	par_cnt++;
		//	continue;
		//}
		//if ((md_lines.dash[i] != 0) | (md_lines.star[i] != 0)) {//这一行是无序排列的开头
		//	if (pre <= i - 1) {
		//		par[par_cnt].start = pre;
		//		par[par_cnt].end = i - 1;
		//		par[par_cnt+1].uolist = (md_lines.dash[i] != 0) ? md_lines.dash[i] : md_lines.star[i];
		//		par_cnt++;
		//	}
		//	pre = i;
		//	
		//	continue;
		//}
		//if (md_lines.number[i] != 0) {//这一行是有序排列的开头
		//	par[par_cnt].start = pre;
		//	par[par_cnt].end = i - 1;
		//	par[par_cnt+1].olist = md_lines.number[i];
		//	pre = i;
		//	par_cnt++;
		//	continue;
		//}
		//if (md_lines.blanks[i] >= 2) {//本行末尾有多个空格
		//	par[par_cnt].start = pre;
		//	par[par_cnt].end = i;
		//	par[par_cnt].blanks = md_lines.blanks[i];
		//	pre = i + 1;
		//	par_cnt++;
		//	continue;
		//}
		//if (md_lines.array[i] == "") {//这一行是空行
		//	par[par_cnt].start = pre;
		//	par[par_cnt].end = i - 1;
		//	pre = i;
		//	par_cnt++;
		//	continue;
		//}

		//标题行
		if (md_lines.title[i] != 0) {
			//把上一段放进数组里
			if (pre <= i - 1) {//之前有多行，连接
				par[par_cnt].start = pre;
				par[par_cnt].end = i - 1;
				par_cnt++;
			}
			//else时，pre==i，即上面已经处理好了
			//把这一段也放进数组里，因为标题段是确定的
			par[par_cnt].start = i;
			par[par_cnt].end = i;
			par[par_cnt].title = md_lines.title[i];
			par_cnt++;
			pre = i + 1;
		}

		//无序列表
		if ((md_lines.dash[i] != 0) | (md_lines.star[i] != 0)) {
			//把前面的一段放进数组
			if (pre <= i - 1) {
				par[par_cnt].start = pre;
				par[par_cnt].end = i - 1;
				par_cnt++;
			}
			//else:pre==i,已经处理好
			//处理新一段的参数
			par[par_cnt].uolist = (md_lines.dash[i] != 0) ? md_lines.dash[i] : md_lines.star[i];
			pre = i;
		}

		//有序列表
		if (md_lines.number[i] != 0) {
			//把前面一段放进数组
			if (pre <= i - 1) {
				par[par_cnt].start = pre;
				par[par_cnt].end = i - 1;
				par_cnt++;
			}
			//处理新一段的参数
			par[par_cnt].olist = md_lines.number[i];
			pre = i;
		}

		//行尾空格
		if (md_lines.blanks[i] >= 2) {
			//把前面一段放进数组
			if (pre <= i) {
				par[par_cnt].start = pre;
				par[par_cnt].end = i;
				par_cnt++;
			}
			//else:说明这一行同时也是标题行，可以不管了
			//下一段参数处理
			pre = i + 1;
		}

		//空行
		if (md_lines.array[i] == "") {
			//把前面一段放进数组
			if (pre <= i - 1) {
				par[par_cnt].start = pre;
				par[par_cnt].end = i - 1;
				par_cnt++;
			}
			par[par_cnt].start = i;
			par[par_cnt].end = i;
			par[par_cnt].title = 1;
			par_cnt++;
			//设置后一段参数
			pre = i + 1;
		}
	}
}

void Trans::getParagraphString() {
	for (int i = 0; i < par_cnt; i++) {
		for (int j = par[i].start; j <= par[i].end; j++) {
			par[i].str += md_lines.array[j];
		}
		par[i].str = solve(par[i].str);
		//cout << par[i].str<<endl;
	}
}

string Trans::title(int i) {
	return  "<h" + to_string(par[i].title) + ">" + par[i].str + "</h" + to_string(par[i].title) + ">";
}


string Trans::list(int m, int n) {//两标题之间的段处理

	//cout << m << ' ' << n << endl;
	if (m > n) return "";//相邻的标题行
	llist list;
	for (int i = m; i <= n; i++) {
		list.add_node(&par[i]);
	}
	string ans = list.solve_list();
	return ans;


}


string Trans::getHTMLLines() {
	string ans;

	int i = 0;
	int title_count = 0;
	while (i < par_cnt) {

		//cout << i <<' '<< par_cnt<< endl;
		if (par[i].title != 0) {//is title
			if (title_count == 0) {
				ans += list(0, i - 1);
			}
			title_count++;
			ans += title(i);

			//cout << html_lines.array[i] << endl;
			int j = i + 1;
			int flag = 0;
			for (; j < par_cnt; j++) {
				if (par[j].title != 0) {

					ans += list(i + 1, j - 1);
					/// process i+1 .. j-1

					i = j;//跳到下一个标题行
					flag = 1;
					break;
					//if (j!=i+1) {
					//	ans += list(i + 1, j - 1);
					//	/// process i+1 .. j-1

					//	i = j;//跳到下一个标题行
					//	break;
					//}
					//else {//两个相邻的标题行
					//	
					//}

				}
				//cout << j<<endl;
			}
			if (flag == 0) {
				ans += list(i + 1, par_cnt - 1);
				break;
			}

		}
		else i++;
	}
	return ans;

}


void Trans::output(string file) {
	//cout<<"start output"<<endl;
	fstream fout(file.c_str(), ios::out);
	fout << "<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"utf-8\">\n<title>my markdown file</title>\n</head>\n<body>\n"
		<< getHTMLLines() << "</body>\n</html>\n";
	fout.close();
	string cmd = "explorer " + file;
	system(cmd.c_str());
}


int main() {
	string inputfile;
	string outputfile;
	cout << "input file name:" << endl;
	cin >> inputfile;

	cout << "output file name:" << endl;
	cin >> outputfile;

	Trans trans;
	trans.input(inputfile);
	//cout << trans.break_ins.cnt << endl;
	//cout << trans.md_file << endl;
	//cout<<trans.solve(trans.md_file);
	trans.getRawLines();
	/*for (int i = 0; i < trans.md_lines.lines_cnt; i++) {
		cout << "i=" << i << endl;
		cout << trans.md_lines.array[i]<<endl;
		cout << "title:  " << trans.md_lines.title[i] << endl;
		cout << "dash:  "<<trans.md_lines.dash[i] << endl;
		cout << "star:  " << trans.md_lines.star[i] << endl;
		cout << "number:  " << trans.md_lines.number[i] << endl;
		cout << "blanks:  " << trans.md_lines.blanks[i] << endl;
		cout << "---------" << endl;
	}
	cout << "---------------------------------------------" << endl;*/
	trans.getParagraph();
	trans.getParagraphString();
	/*for (int i = 0; i < trans.par_cnt; i++) {
		cout << "start: "<<trans.par[i].start << endl;
		cout << "end: " << trans.par[i].end << endl;
		cout << "title: " << trans.par[i].title << endl;
		cout << "olist: " << trans.par[i].olist << endl;
		cout << "uolist: " << trans.par[i].uolist << endl;
		cout << "blanks: " << trans.par[i].blanks << endl;
		cout << "---------" << endl;
	}*/
	trans.output(outputfile);
	//cout<<"/////////////////////////\n"<<trans.getHTMLLines() << "/////////////////////////\n";
}