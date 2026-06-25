//
// Created by TX on 2026/6/19.
//

#include "TypewriterLabel.h"

#include "Core/SizeManager/SizeManager.h"

TypewriterLabel::TypewriterLabel(QWidget *parent) :
    QLabel(parent)
{
    setAlignment(Qt::AlignmentFlag::AlignCenter);
    connect(&_timer, &QTimer::timeout, this, &TypewriterLabel::tick);
    connect(SizeManager::manager(), &SizeManager::sizeAdjustmentCompleted, this, &TypewriterLabel::reSetFontSize);
    setTexts({
            "独行独坐，独唱独酬还独卧——朱淑真·减字木兰花",
            "孤舟蓑笠翁，独钓寒江雪——柳宗元·江雪",
            "十年生死两茫茫，不思量，自难忘——苏轼·江城子",
            "落花人独立，微雨燕双飞——晏几道·临江仙",
            "当时明月在，曾照彩云归——晏几道·临江仙",
            "人生若只如初见，何事秋风悲画扇——纳兰性德·木兰花令",
            "何处合成愁，离人心上秋——吴文英·唐多令",
            "问君能有几多愁，恰似一江春水向东流——李煜·虞美人",
            "夕阳无限好，只是近黄昏——李商隐·登乐游原",
            "自在飞花轻似梦，无边丝雨细如愁——秦观·浣溪沙",
            "劝君更尽一杯酒，西出阳关无故人——王维·送元二使安西",
            "念桥边红药，年年知为谁生——姜夔·扬州慢",
            "试问闲愁都几许？一川烟草，满城风絮——贺铸·青玉案",
            "日日思君不见君,共饮长江水——李之仪·卜算子·我住长江头",
            "叶上初阳干宿雨，水面清圆，一一风荷举——周邦彦·苏幕遮",
            "我是人间惆怅客，知君何事泪纵横——纳兰性德·浣溪沙",
            "枯藤老树昏鸦，小桥流水人家——马致远·天净沙·秋思",
            "念天地之悠悠，独怆然而涕下——陈子昂·登幽州台歌",
            "人面不知何处去，桃花依旧笑春风——崔护·题都城南庄",
            "悲欢离合总无情，一任阶前点滴到天明——蒋捷·虞美人",
        }
    );
}

void TypewriterLabel::setTexts(const QStringList &texts)
{
    _timer.stop();
    _texts = texts;
    _textIdx = 0;
    _charIdx = 0;
    _state = State::Type;
    setText(QString());
}

void TypewriterLabel::setSpeed(const int typeMs, const int deleteMs, const int pauseMs)
{
    _typeMs = typeMs;
    _deleteMs = deleteMs;
    _pauseMs = pauseMs;
}

void TypewriterLabel::start()
{
    if (_texts.isEmpty())
        return;
    _timer.start(_typeMs);
}

void TypewriterLabel::stop()
{
    _timer.stop();
}

void TypewriterLabel::skip()
{
    if (_texts.isEmpty())
        return;
    _textIdx = (_textIdx + 1) % _texts.size();
    _charIdx = 0;
    _state = State::Type;
    _timer.start(_typeMs);
    setText(QString());
}

// ─────────────────────────────────────────────
// 状态机核心：Type → Pause → Delete → Type ...
// ─────────────────────────────────────────────
void TypewriterLabel::tick()
{
    if (_texts.isEmpty())
    {
        _timer.stop();
        return;
    }

    const QString &cur = _texts[_textIdx];

    switch (_state)
    {
        case State::Type :
            if (_charIdx < cur.size())
            {
                ++_charIdx;
                setText(cur.left(_charIdx));
                _timer.start(_typeMs);
            }
            else
            {
                // 打字完成 → 暂停（仅多条文本时进入删除循环）
                if (_texts.size() > 1)
                {
                    _state = State::Pause;
                    _timer.start(_pauseMs);
                }
                // 单条文本：保持显示，不删除
            }
            break;

        case State::Pause :
            _state = State::Delete;
            _timer.start(_deleteMs);
            break;

        case State::Delete :
            if (_charIdx > 0)
            {
                --_charIdx;
                setText(cur.left(_charIdx));
                _timer.start(_deleteMs);
            }
            else
            {
                // 删除完成 → 下一条
                _state = State::Type;
                _textIdx = (_textIdx + 1) % _texts.size();
                _timer.start(_typeMs);
            }
            break;
    }
}

void TypewriterLabel::reSetFontSize()
{
    const int vw = qRound(window()->width() / window()->devicePixelRatio());
    const double s = vw > 1280 ? qBound(0.85, vw / 2000.0, 1.0) : 1.0;
    const int subPx = vw >= 1280 ? qRound(30.0 * s) : vw >= 768 ? qRound(24.0 * s) : vw >= 480 ? 16 : 14;
    
    QFont f1 = font();
    f1.setPixelSize(subPx);
    setFont(f1);
}
