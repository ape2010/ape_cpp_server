#include "timermanager.h"
#include "threadtimer.h"
#include "loghelper.h"
#include <sys/time.h>

namespace ape{
namespace common{
static uint64_t GetCurrentMillisec() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return (uint64_t)tp.tv_sec * 1000 + tp.tv_usec / 1000;
}
CTimerManager::CTimerManager() {
    checktime_ = GetCurrentMillisec();
    wheels_[0] = new SWheel(WHEEL_SIZE1);
    for (int i = 1; i < WHEEL_NUM; ++i) {
        wheels_[i] = new SWheel(WHEEL_SIZE2);
    }
}
CTimerManager::~CTimerManager() {
    for (int i = 0; i < WHEEL_NUM; ++i) {
        if (wheels_[i]) {delete wheels_[i]; wheels_[i] = NULL;}
    }
}
void CTimerManager::DetectTimerList() {
    uint64_t now = GetCurrentMillisec();
    uint32_t loopnum = now > checktime_ ? (now - checktime_) / GRANULARITY : 0;

    SWheel *wheel =  wheels_[0];
    //BS_XLOG(XLOG_DEBUG, "CTimerManager::%s, loopnum[%u], spokeindex[%u]\n", __FUNCTION__, loopnum, wheel->spokeindex);
    for (uint32_t i = 0; i < loopnum; ++i) {
        SNodeLink *spoke = wheel->spokes + wheel->spokeindex;
        SNodeLink *link = spoke->next;
        while (link != spoke) {
            STimerNode *node = (STimerNode *)link;
            link->prev->next = link->next;
            link->next->prev = link->prev;
            link = node->link.next;
            AddToReadyNode(node);
        }
        if (++(wheel->spokeindex) >= wheel->size) {
            wheel->spokeindex = 0;
            Cascade(1);
        }
        checktime_ += GRANULARITY;
    }
    DoTimeOutCallBack();
}
void CTimerManager::AddToReadyNode(STimerNode *node) {
    SNodeLink *nodelink = &(node->link);
    nodelink->prev = readynodes_.prev;
    readynodes_.prev->next = nodelink;
    nodelink->next = &readynodes_;
    readynodes_.prev = nodelink;
}
void CTimerManager::DoTimeOutCallBack() {
    SNodeLink *link = readynodes_.next;
    //if (link != &readynodes_) {Dump();}
    while (link != &readynodes_) {
        STimerNode *node = (STimerNode *)link;
        node->timer->Callback();
        link = node->link.next;
        delete node;
    }
    readynodes_.next = readynodes_.prev = &readynodes_;
}
uint32_t CTimerManager::Cascade(uint32_t wheelindex) {
    //BS_XLOG(XLOG_DEBUG, "CTimerManager::%s, wheelindex[%u]\n", __FUNCTION__, wheelindex);
    if (wheelindex < 1 || wheelindex >= WHEEL_NUM) {
        return 0;
    }
    SWheel *wheel =  wheels_[wheelindex];
    int casnum = 0;
    uint64_t now = GetCurrentMillisec();
    SNodeLink *spoke = wheel->spokes + (wheel->spokeindex++);
    SNodeLink *link = spoke->next;
    //BS_XLOG(XLOG_TRACE, "CTimerManager::%s::%d, wheelindex[%u], spokeindex[%u], link[%p], spoke[%p]\n",
    //    __FUNCTION__, __LINE__, wheelindex, wheel->spokeindex, link, spoke);
    spoke->next = spoke->prev = spoke;
    //BS_XLOG(XLOG_TRACE, "CTimerManager::%s::%d, wheelindex[%u], spokeindex[%u], link[%p], spoke[%p]\n",
    //    __FUNCTION__, __LINE__, wheelindex, wheel->spokeindex, link, spoke);
    while (link != spoke) {
        STimerNode *node = (STimerNode *)link;
        link = node->link.next;
        if (node->dead_time <= now) {
            AddToReadyNode(node);
        } else {
            uint32_t milseconds = node->dead_time - now;
            //BS_XLOG(XLOG_DEBUG, "CTimerManager::%s, wheelindex[%u], link[%p], milseconds[%u]\n", __FUNCTION__, wheelindex, link, milseconds);
            AddTimerNode(milseconds, node);
            ++casnum;
        }

    }

    if (wheel->spokeindex >= wheel->size) {
        wheel->spokeindex = 0;
        casnum += Cascade(++wheelindex);
    }
    return casnum;
}
STimerNode* CTimerManager::AddTimer(uint32_t milseconds, CThreadTimer *timer) {
    uint64_t dead_time = GetCurrentMillisec() + milseconds;
    STimerNode *node = new STimerNode(timer, dead_time);
    AddTimerNode(milseconds, node);
    return node;
}
void CTimerManager::AddTimerNode(uint32_t milseconds, STimerNode *node) {
    SNodeLink *spoke = NULL;
    uint32_t interval = milseconds / GRANULARITY;
    uint32_t threshold1 = WHEEL_SIZE1;
    uint32_t threshold2 = 1 << (WHEEL_BITS1 + WHEEL_BITS2);
    uint32_t threshold3 = 1 << (WHEEL_BITS1 + 2 * WHEEL_BITS2);
    uint32_t threshold4 = 1 << (WHEEL_BITS1 + 3 * WHEEL_BITS2);

    if (interval < threshold1) {
        uint32_t index = (interval + wheels_[0]->spokeindex) & WHEEL_MASK1;
        spoke = wheels_[0]->spokes + index;
    } else if (interval < threshold2) {
        uint32_t index = ((interval - threshold1 + wheels_[1]->spokeindex * threshold1) >> WHEEL_BITS1) & WHEEL_MASK2;
        spoke = wheels_[1]->spokes + index;
    } else if (interval < threshold3) {
        uint32_t index = ((interval - threshold2 + wheels_[2]->spokeindex * threshold2) >> (WHEEL_BITS1 + WHEEL_BITS2)) & WHEEL_MASK2;
        spoke = wheels_[2]->spokes + index;
    } else if (interval < threshold4) {
        uint32_t index = ((interval - threshold3 + wheels_[3]->spokeindex * threshold3) >> (WHEEL_BITS1 + 2 * WHEEL_BITS2)) & WHEEL_MASK2;
        spoke = wheels_[3]->spokes + index;
    } else {
        uint32_t index = ((interval - threshold4 + wheels_[4]->spokeindex * threshold4) >> (WHEEL_BITS1 + 3 * WHEEL_BITS2)) & WHEEL_MASK2;
        spoke = wheels_[4]->spokes + index;
    }
    SNodeLink *nodelink = &(node->link);
    nodelink->prev = spoke->prev;
    spoke->prev->next = nodelink;
    nodelink->next = spoke;
    spoke->prev = nodelink;
}
void CTimerManager::RemoveTimer(STimerNode* node) {
    //BS_XLOG(XLOG_DEBUG, "CTimerManager::%s, node[%p], dead_time[%lld]\n", __FUNCTION__, node, node->dead_time);
    SNodeLink *nodelink = &(node->link);
    if (nodelink->prev) {
        nodelink->prev->next = nodelink->next;
    }
    if (nodelink->next) {
        nodelink->next->prev = nodelink->prev;
    }
    nodelink->prev = nodelink->next = NULL;

    delete node;
}
void CTimerManager::Dump() {
    BS_XLOG(XLOG_DEBUG, "CTimerManager::%s ===== \n", __FUNCTION__);
    for (int i = 0; i < WHEEL_NUM; ++i) {
        SWheel *wheel = wheels_[i];
        BS_XLOG(XLOG_DEBUG, "    wheel[%d].size[%u], spokeindex[%u]:\n", i, wheel->size, wheel->spokeindex);
        for (int j = 0; j < wheel->size; ++j) {
            SNodeLink *spoke = wheel->spokes + j;
            SNodeLink *link = spoke->next;
            if (link != spoke) {
                BS_XLOG(XLOG_DEBUG, "       spoke index[%d], addr[%p], next[%p], prev[%p]\n", j, spoke, spoke->next, spoke->prev);
            }
            while (link != spoke) {
                STimerNode *node = (STimerNode *)link;
                BS_XLOG(XLOG_DEBUG, "          node[%p], next[%p], prev[%p] interval[%u], dead_time[%lld]\n",
                    link, link->next, link->prev, node->timer->GetInterval(), node->dead_time);
                link = node->link.next;
            }
        }
    }
}

}
}