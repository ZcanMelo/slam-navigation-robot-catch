#!/usr/bin/env python3 
# -*- coding: utf-8 -*-
from snownlp import seg
from rank_bm25 import BM25Okapi,BM25Plus,BM25L
import numpy as np
import json

class BM25:
    def __init__(self,file_path = '/home/handsfree/handsfree/handsfree_ros_ws/src/handsfree/robot/script/load_file.txt') -> None:
        
        self.file_path = file_path
        # self.stopword_list = self.load_stop()
        self.docs_list = self.seg_docs()
        
    def seg_docs(self):
        docs = []
        source_data = []
        with open(self.file_path, 'r',encoding="utf-8") as f:
            i = 0
            for line in f: 
                i += 1
                line = line.strip()
                if line.startswith("filepath="):
                    continue
                #print(line)
                docs.append(line[line.index("'")+1: line.index('metadata')-2])
                source_data.append(eval(line[line.index("metadata")+9:]))
        self.docs = docs
        self.source_data = source_data

        docs_list = []
        for sentence in self.docs:
            word_list = seg.seg(sentence) 
            # word_list = self.filter_stop(word_list)
            docs_list.append(word_list)
        return docs_list

    # def load_stop(self, file = '/data/gmlabBM25/hit_stopwords.txt'):
    #     with open(file, 'r', encoding='utf-8') as f:    
    #         stopword_list = [word.strip('\n') for word in f.readlines()]
    #     return stopword_list


    # def filter_stop(self, words):
    #     return list(filter(lambda x: x not in self.stopword_list, words))

    def get_context(self, query, top_k = 2):
        # 对query进行分词
        tokenized_query = seg.seg(query) 
        bm25 = BM25Plus(self.docs_list)
        doc_scores = bm25.get_scores(tokenized_query)

        sorted_indices = np.argsort(doc_scores)[::-1]

        top_n_indices = sorted_indices[:top_k]

        context = np.array(self.docs)[top_n_indices]

        #rerank add here
        # import reRanking
        # reRanked_docs = reRanking.reRanker(query,context)

        context = "\n".join(context)

        source_dict = {}
        dictidx = 0
        for indics in top_n_indices:
            source_dict[f'title{dictidx}'] = self.source_data[indics]['title'] 
            source_dict[f'picture{dictidx}'] = self.source_data[indics]['picture'] 
            dictidx +=1
        source_json = json.dumps(source_dict, ensure_ascii=False)
        return context, source_json

#!/usr/bin/env python
#coding=UTF-8
from std_msgs.msg import String
import rospy

#nav_goal = rospy.Publisher("nav_goal", String, queue_size = 5)
if __name__ == '__main__':
    rospy.init_node('bm25')
    file_path = '/home/handsfree/handsfree/handsfree_ros_ws/src/handsfree/robot/script/utils/load_file.txtt'
    BM = BM25(file_path = file_path)
    question = """在三星堆文化中，跪坐人像确实具有一定的宗教或仪式意义。三星堆遗址位于四川省广汉市，距今约4500-2800年，是中国古代文明的重要代表之一。在这些出土文物中，有大量精美的青铜器、玉器和陶器等。其中，最具代表性的就是那些跪坐人像。

这些跪坐人像通常呈坐姿，双手合十，面朝前方，双腿并拢。它们的面部表情庄重而神秘，给人一种庄严肃穆的感觉。这种形象在古代中国的宗教和艺术中具有特殊的象征意义。在古代中国，跪拜是一种表示尊敬、敬意和虔诚的礼仪，人们通过跪拜来祈求神灵的庇佑和保护。因此，这些跪坐人像可能是用于祭祀仪式的道具，代表着人们对神灵的敬畏和信仰。

此外，这些跪坐人像还可能与当时的社会等级制度有关。在古代中国，身份地位较高的人物通常被描绘成跪坐的姿势，以示尊重和谦卑。因此，这些跪坐人像也可能是当时社会中上层人物的象征。

总之，在三星堆文化中，跪坐人像具有特殊的宗教和仪式意义，它们可能是用于祭祀活动的工具，也反映了当时社会的等级制度。青铜跪坐人像"""
    context, source_json = BM.get_context(query=question, top_k = 1)
    sj = json.loads(source_json)
    for k, v in list(sj.items()):
    	   rospy.loginfo(k)
    	   rospy.loginfo(v)
#        print(k, v)
    # rospy.spin()

    