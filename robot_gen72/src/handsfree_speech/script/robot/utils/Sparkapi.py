#!/usr/bin/env python3 
# -*- coding: utf-8 -*-

import _thread as thread
import base64
import datetime
import hashlib
import hmac
import json
from urllib.parse import urlparse
import ssl
from datetime import datetime
from time import mktime
from urllib.parse import urlencode
from wsgiref.handlers import format_date_time

import websocket

answer = ''

class Ws_Param(object):
    # 初始化
    def __init__(self, APPID, APIKey, APISecret, gpt_url):
        self.APPID = APPID
        self.APIKey = APIKey
        self.APISecret = APISecret
        self.host = urlparse(gpt_url).netloc
        self.path = urlparse(gpt_url).path
        self.gpt_url = gpt_url

    # 生成url
    def create_url(self):
        # 生成RFC1123格式的时间戳
        now = datetime.now()
        date = format_date_time(mktime(now.timetuple()))

        # 拼接字符串
        signature_origin = "host: " + self.host + "\n"
        signature_origin += "date: " + date + "\n"
        signature_origin += "GET " + self.path + " HTTP/1.1"

        # 进行hmac-sha256进行加密
        signature_sha = hmac.new(self.APISecret.encode('utf-8'), signature_origin.encode('utf-8'),
                                 digestmod=hashlib.sha256).digest()

        signature_sha_base64 = base64.b64encode(signature_sha).decode(encoding='utf-8')

        authorization_origin = f'api_key="{self.APIKey}", algorithm="hmac-sha256", headers="host date request-line", signature="{signature_sha_base64}"'

        authorization = base64.b64encode(authorization_origin.encode('utf-8')).decode(encoding='utf-8')

        # 将请求的鉴权参数组合为字典
        v = {
            "authorization": authorization,
            "date": date,
            "host": self.host
        }
        # 拼接鉴权参数，生成url
        url = self.gpt_url + '?' + urlencode(v)
        # 此处打印出建立连接时候的url,参考本demo的时候可取消上方打印的注释，比对相同参数时生成的url与自己代码生成的url是否一致
        return url


# 收到websocket错误的处理
def on_error(ws, error):
    print(("### error:", error))

# 收到websocket关闭的处理
def on_close(ws, close_status_code, close_msg):
    print("### closed ###")


# 收到websocket连接建立的处理
def on_open(ws):
    thread.start_new_thread(run, (ws,))


def run(ws, *args):
    data = json.dumps(gen_params(appid=ws.appid, question=ws.question))
    ws.send(data)


# 收到websocket消息的处理
def on_message(ws, message):
    data = json.loads(message)
    code = data['header']['code']
    if code != 0:
        print(f'请求错误: {code}, {data}')
        ws.close()
    else:
        choices = data["payload"]["choices"]
        status = choices["status"]
        content = choices["text"][0]["content"]
        global answer
        answer += content
        #print(content, end='')
        if status == 2:
            global tokens
            tokens = data['payload']['usage']
            ws.close()


def gen_params(appid, question):
    """
    通过appid和用户的提问来生成请参数
    """
    data = {
        "header": {
            "app_id": appid,
            "uid": "1234"
        },
        "parameter": {
            "chat": {
                "domain": "generalv2",
                "random_threshold": 0.5,
                "max_tokens": 4096,
                "auditing": "default"
            }
        },
        "payload": {
            "message": {
                "text": [
                    {"role": "user", "content": question}
                ]
            }
        }
    }
    return data

#根据tet5 和得到的回复res: [{"role": "assistant", "content": "在三星堆遗址中，考古学家们发现了许多重要的文物和遗迹。其中最著名的是青铜神树，这是世界上最大的青铜器之一，高约3米，有69个分支。此外，还有象形文字、玉器、陶器等珍贵的文物。这些发现为我们了解古代蜀国的社会、宗教和艺术提供了宝贵的信息。"}, {"role": "user", "content": "三星堆博物馆里有哪些展品？"}]，编写多轮对话函数multi_QA
def sparkllm(question, appid="bce95c07",
         api_key="2b89ab4280f091d2c03b19a236f034ac",
         api_secret="MThjYjZlYjM2YzAxZDQ1ZmE5ODY4NjU0",
         gpt_url="wss://spark-api.xf-yun.com/v2.1/chat"):#wss://aichat.xf-yun.com/v1/chat
    wsParam = Ws_Param(appid, api_key, api_secret, gpt_url)
    websocket.enableTrace(False)
    wsUrl = wsParam.create_url()
    ws = websocket.WebSocketApp(wsUrl, on_message=on_message, on_error=on_error, on_close=on_close, on_open=on_open)
    ws.appid = appid
    ws.question = question
    ws.run_forever(sslopt={"cert_reqs": ssl.CERT_NONE})
    return answer


def multi_QA(guestQ, history):
    guestq = f"{guestQ}"
    historyjs = json.loads(history)
    historyjs+=[{"role": "user", "content": guestq}]
    tet6 = history[:-2]
    tet6 += ',\n'
    tet6 += json.dumps({"role": "user", "content": guestq}, ensure_ascii = False)
    tet6 += '\n]'
    print(tet6)
    res = sparkllm(question=tet6)
    print(res)
    jsres = json.loads(res.strip())
    if type(jsres) != list:
        jsres = [jsres]
    for response in jsres:
        if response['role'] == 'assistant':
            resA=response['content']
            historyjs += [response]
            print(resA)
    historynew = """["""
    for item in historyjs:
        historynew += '\n'
        historynew += json.dumps(item, ensure_ascii = False)
        historynew += ','
    historynew = historynew[:-1]
    historynew += '\n]'
    return resA,historynew

def multi_language(resraw, lang="英语"):
    #resraw必须是单轮的字符串
    resraw += f"翻译成{lang}"
    return sparkllm(question=resraw)

def env_perp(environment_description, task, ins):
    # environment_description = "现在摄像头发现前方20cm有个冰箱,其他地方空白"
    # task = "找到饼干"
    # ins = '''{"目标":"饼干","任务列表": [{"动作": "U 0.5", "描述": "向前走0.5米，确认冰箱位置"}, {"动作": "L 0.3", "描述": "向左走0.3米，在冰箱附近寻找饼干"}, {"动作": "R 0.6", "描述": "向右走0.6米，成功找到饼干"}],"是否完成任务":"否"}'''
    inj = json.loads(ins)
    ques = f'现在环境是{environment_description.strip()},任务描述是{task.strip()}请判断是否完成任务,并根据指令列表{inj}返回下一个状态的任务列表.结果仅为json格式'
    return ques

def introduc_from_materi(objname, objcontent):
    # objcontent = "一个身高1.8米的青铜人像，站立在高达0.8米的台座上。这是三星堆古国最有代表性的器物之一，也是举世罕见的古文明大型青铜人像。铜人连同底座高达2.62米，基座高0.9米，人像高1.72米，铜像中空，重约180公斤。铜像五官突出，棱角分明，耳垂上有圆形耳洞。铜人身着三层薄衣，内层长至小腿，后摆开叉，衣服上有精美的花纹。此铜像左右手中心轴不在一条直线上。有的学者认为这样设计是用来放置和展示象牙或瑞草。\n\n青铜立人像出土时从腰部被断为两截，背部断裂变形，专家推测在入土前就被砸碎，1987年8月由当时中国历史博物馆专家孙振翔、赵家英、傅金凯、王赴朝等人修复。修复后背后缺少一个铜片，后在1993年，三星堆文物修复完成后的清理中找到该块残片。"
    # objname = "青铜大立人像"
    tet = f"请根据材料：{objcontent}，介绍一下{objname}"
    return tet

def normal_identif_setting(guest, museum_name="三星堆博物馆"):
    # guest= "古蜀文明是什么"
    # museum_name = "三星堆博物馆"
    tet = f"你现在是{museum_name}导游，游客对你说“{guest}”"
    return tet

def action_point(guest, pool_poi, current_poi):
    tet = f"游客说[{guest}],博物馆有相关的位置有[{pool_poi}],请你从下面选一个和游客内容相关的位置"
    return tet

if __name__ == "__main__":
    # 测试时候在此处正确填写相关信息即可运行
    #sparkllm(question="一辆20*30*10cm的科研小车，现在摄像头发现前方20cm有个冰箱，你能向我展示你知不知道这样的情况路径怎么走吗")
    environment_description = "现在摄像头发现前方20cm有个冰箱,其他地方空白"
    task = "找到饼干"
    ins = '''{"目标":"饼干","任务列表": [{"动作": "U 0.5", "描述": "向前走0.5米，确认冰箱位置"}, {"动作": "L 0.3", "描述": "向左走0.3米，在冰箱附近寻找饼干"}, {"动作": "R 0.6", "描述": "向右走0.6米，成功找到饼干"}],"是否完成任务":"否"}'''
    inj = json.loads(ins)

    print((inj['任务列表'][0]['动作']))

    # ques = f'现在环境是{environment_description.strip()}请根据任务{task.strip()}以思维链方式返回一个任务列表json格式'
    ques = f'现在环境是{environment_description.strip()},任务描述是{task.strip()}请判断是否完成任务,并根据指令列表{inj}返回下一个状态的任务列表.结果仅为json格式'
    tues = """Imagine I am a robot equipped with a camera and a depth sensor. I am trying to perform a task, and you should help me by sending me commands. You are only allowed to give me the following commands:

turn(angle): turn the robot by a given number of degrees
move(distance): moves the robot straight forward by a given distance in meters.
On each step, I will provide you with the objects in the scene as a list of <object name, distance, angle in degrees>. You should reply with only one command at a time.
The distance is in meters, and the direction angle in degrees with respect to the robot's orientation. Negative angles are to the left and positive angles are to the right. If a command is not valid, I will ignore it and ask you for another command. If there is no relevant information in the scene, use the available commands to explore the environment.


eg：
Task: go to the chairs
Objects in the scene: <door, 0.53 m, 22>, <chair, 4.84 m, -21>, <chair, 2.12 m, -24>, <window, 6.74 m, -3>, <window, 7.17 m, 6>, <door, 0.61 m, 18>, <stairs, 1.42 m, 0>
Command:

move(1.5) # Move forward by 1.5 meters to explore the environment. 

task:
find a chair
Objects in the scene: <seating, 5.05 m, 21>, <chair, 3.93 m, -25>, <window, 4.55 m, 22>, <window, 5.24 m, -4>, <stairs, 3.49 m, 13>, <window, 5.67 m, 7>, <window, 4.37 m, 23>
command:"""
    #tet = "深圳市有几个行政区"
    objcontent = "一个身高1.8米的青铜人像，站立在高达0.8米的台座上。这是三星堆古国最有代表性的器物之一，也是举世罕见的古文明大型青铜人像。铜人连同底座高达2.62米，基座高0.9米，人像高1.72米，铜像中空，重约180公斤。铜像五官突出，棱角分明，耳垂上有圆形耳洞。铜人身着三层薄衣，内层长至小腿，后摆开叉，衣服上有精美的花纹。此铜像左右手中心轴不在一条直线上。有的学者认为这样设计是用来放置和展示象牙或瑞草。\n\n青铜立人像出土时从腰部被断为两截，背部断裂变形，专家推测在入土前就被砸碎，1987年8月由当时中国历史博物馆专家孙振翔、赵家英、傅金凯、王赴朝等人修复。修复后背后缺少一个铜片，后在1993年，三星堆文物修复完成后的清理中找到该块残片。"
    objname = "青铜大立人像"
    tet = f"请根据材料：{objcontent}，介绍一下{objname}，用西班牙语"

    guestQA = "我想看那个戴黄金面具的青铜像"
    tet2 = f"游客想去参观一个文物，他对你说{guestQA}，他想参观的文物名称是？"
    
    museum_name = "三星堆博物馆"
    single_round_guest = "博物馆里的铜摇钱树在哪"
    tet3 = f"你现在是{museum_name}导游，游客对你说“{single_round_guest}”。如果游客有提到和{museum_name}内有关的物体或物体描述，请只输出游客原话中对应的物体或物体描述"

    # guest = "你能告诉我关于三星堆的历史和文化背景吗？"
    guest= "古蜀文明是什么"
    tet4 = f"你现在是{museum_name}导游，游客对你说“{guest}”"


    #多轮对话
    
    tet5 = """[ {"role": "assistant", "content": "您好，欢迎来到三星堆博物馆！我是您的导游，很高兴能为您提供导览服务。"}, {"role": "user", "content": "你能告诉我关于三星堆的历史和文化背景吗？"}, {"role": "assistant", "content": "当然！三星堆是一处古代文明遗址，距今约4000多年，曾是古蜀国的一部分。这里出土了许多珍贵的文物，反映了当时的社会、宗教和艺术。我们将在博物馆中看到一些重要的展品。"}]"""
    # his = """[{"role": "user", "content": "你好"}, 
    # {"role": "assistant", "content": "您好，欢迎来到三星堆博物馆！我是您的导游，很高兴能为您提供导览服务。"}, 
    # {"role": "user", "content": "你能告诉我关于三星堆的历史和文化背景吗？"}, 
    # {"role": "assistant", "content": "当然！三星堆是一处古代文明遗址，距今约4000 多年，曾是古蜀国的一部分。这里出土了许多珍贵的文物，反映了当时的社会、宗教和艺术。我们将在博物馆中看到一些重要的展品。"}, 
    # {"role": "user", "content": "有什么重要的考古发现在这里呢？"}, 
    # {"role": "assistant", "content": "在三星堆遗址中，出土了许多重要的考古发现，其中最著名的是青铜神树。这是一棵高达3米的神树，由青铜制成，上面镶嵌着象牙、玉石和珍珠等珍贵材料。此外，这里还出土了青铜面具、青铜立人像、陶器等文物，这些都反映了当时的社会、宗教和艺术。"}]"""
    # res_str = sparkllm(question=tet5)
    # print("res: "+res_str)
    # print("\n\n\nround one\n")
    # guestq = "有什么重要的考古发现在这里呢？"
    # res,his = multi_QA(guestq,tet5)
    # print(his)
   
    tet7 = """[
{"role": "user", "content": "你好"},
{"role": "assistant", "content": "您好，欢迎来到三星堆博物馆！我是您的导游，很高兴能为您提供导览服务。"}, 
{"role": "user", "content": "你能告诉我关于三星堆的历史和文化背景吗？"}, 
{"role": "assistant", "content": "当然！三星堆是一处古代文明遗址，距今约4000 多年，曾是古蜀国的一部分。这里出土了许多珍贵的文物，反映了当时的社会、宗教和艺术。我们将在博物馆中看到一些重要的展品。"}, 
{"role": "user", "content": "有什么重要的考古发现在这里呢？"}, 
{"role": "assistant", "content": "在三星堆遗址中，出土了许多重要的考古发现，包括青铜器、玉器、陶器等。其中最著名的是青铜神树和青铜神面具，它们代表了古代蜀国的宗教信仰和社会文化。此外，还有一些珍贵的玉器，如玉璋、玉琮等，展示了当时的工艺水平和审美观念。这些考古发现为我们了解古蜀国的历史和文化提供了宝贵的信息。"}
]"""
#     tet7 = """[{"role": "assistant", "content": "您好，欢迎来到三星堆博物馆！我是您的导游，很高兴能为您提供导览服务。"}, {"role": "user", "content": "你能告诉我关于三星堆的历史和文化背景吗？"}, {"role": "assistant", "content": "当然！三星堆是一处古代文明遗址，距今约4000 
# 多年，曾是古蜀国的一部分。这里出土了许多珍贵的文物，反映了当时的社会、宗教和艺术。我们将在博物馆中看到一些重要的展品。"}, {"role": "user", "content": "有什么重要的考古发现在这里呢？"}, {"role": "assistant", "content": "在三星堆遗址中，出土了许多重要的考古发
# 现，包括青铜器、玉器、陶器等。其中最著名的是青铜神树和青铜神面具，它们代表了古代蜀国的宗教信仰和社会文化。此外，还有一些珍贵的玉器，如玉璋、玉琮等，展示了当时的工艺水平和审美观念。这些考古发现为我们了解古蜀国的历史和文化提供了宝贵的信息。"}, {"role": "user", "content": "和宗教有什么关系吗"}]
# """
    tet7 = "你现在{青铜大立人像}文物旁,游客说{咱们去看看那个青铜树吧},请你从以下博物馆有相关的位置{青铜大立人像，大型青铜通天神树，三星堆铜鸡}，选一个和游客内容相关的位置"
    # current_poi = "青铜神树"
    # guest = "咱们去看看那个青铜树吧"
    # pool_poi = ["青铜大立人像", "大型青铜通天神树", "三星堆铜鸡"]
    # tet7 = action_point(guest,pool_poi,current_poi)
    tet7 = "你现在[青铜神树]文物旁,游客说[咱们去看看那个青铜树吧],博物馆有相关的位置有[青铜大立人像], [大型青铜通天神树], [三星堆铜鸡],请你从下面选一个和游客内容相关的位置"
    res_str = sparkllm(question=tet7)
    # import bm25
    # bm25.BM.get_context(query=res_str)
    print(("res: "+res_str))

    # print("\n\n\nround twe\n")
    # guestq2 = "和祭祀有什么关系吗"
    # res2,his2 = multi_QA(guestq2,tet7)
    # print(his2)
    # print("\n\n\nround three\n")
    # guestq3 = "明天开馆吗"
    # res3,his3 = multi_QA(guestq3,his2)
    # print(his3)

    #print(json.loads(res_str))
    print(tokens)



# curl -X POST "http://172.16.11.42:8000" \
#      -H 'Content-Type: application/json' \
#      -d '{"prompt": "你好", "history": []}'