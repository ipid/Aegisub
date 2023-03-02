#include "grid_warning_controller.h"

#include <wx/event.h>
#include <wx/msgdlg.h>
#include <vector>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/flyweight.hpp>
#include <algorithm>
#include <chrono>

#include "ass_file.h"
#include "ass_dialogue.h"
#include "include/aegisub/context.h"

constexpr int WARNING_COMMIT_MASK = 0 | AssFile::COMMIT_DIAG_ADDREM | AssFile::COMMIT_DIAG_META | AssFile::COMMIT_DIAG_TIME | AssFile::COMMIT_DIAG_TEXT;
constexpr int LEGACY_A_TO_AN[] = {-1, 1, 2, 3, -1, 7, 8, 9, -1, 4, 5, 6};

GridWarningController::GridWarningController(agi::Context *context)
    : subtitle_commit_connection(context->ass->AddCommitListener(&GridWarningController::OnSubtitlesCommit, this)), context(context)
{
    // TODO: 看看能做啥
}

static bool shouldEventBeConsidered(const AssDialogue &event)
{
    if (boost::iequals(event.Effect.get(), "fx") || boost::starts_with(event.Effect.get(), "Banner;"))
    {
        return false;
    }

    if (event.Comment && event.Effect != "Karaoke" && event.Effect != "karaoke")
    {
        return false;
    }

    if (event.Text.get().find_first_not_of(' ') == std::string::npos)
    {
        return false;
    }

    return true;
}

static DialogueProperties parseEvent(const AssDialogue &event)
{
    std::vector<std::unique_ptr<AssDialogueBlock>> tags = event.ParseTags();

    DialogueProperties prop;
    prop.original = &event;

    for (auto &block_unknown_ptr : tags)
    {
        AssDialogueBlock *block_unknown = block_unknown_ptr.get();

        if (block_unknown->GetType() != AssBlockType::OVERRIDE)
        {
            continue;
        }

        AssDialogueBlockOverride *block = static_cast<AssDialogueBlockOverride *>(block_unknown);
        for (AssOverrideTag &tag : block->Tags)
        {
            if (!tag.IsValid())
            {
                continue;
            }
            if (tag.Name == "move")
            {
                prop.hasMove = true;
            }
            else if (tag.Name == "pos" && prop.posX < 0)
            {
                prop.posX = tag.Params[0].Get<float>();
                prop.posY = tag.Params[1].Get<float>();
            }
            else if (tag.Name == "an")
            {
                prop.align = tag.Params[0].Get<int>();
            }
            else if (tag.Name == "a")
            {
                int legacyAlign = tag.Params[0].Get<int>();

                if (legacyAlign < 0 || legacyAlign > 11)
                {
                    prop.align = -1;
                }
                else
                {
                    prop.align = LEGACY_A_TO_AN[legacyAlign];
                }
            }
        }
    }

    return prop;
}

void GridWarningController::OnSubtitlesCommit(int type, const AssDialogue *line)
{
    /*

    两条字幕同时满足以下所有条件时，被认为是冲突字幕：

        1. 满足下列条件之一：
            a. 两条字幕都是注释，且两条字幕的特效都是 [Kk]araoke
            b. 两条字幕都是对话（不是注释），且两条字幕的特效都不是 fx
        2. 满足下列条件之一：
            a. 两条字幕的样式相同，且样式都不是 Default
            b. 两条字幕的样式都是 Default 且“说话人”一致（都为空也行）
        3. 两条字幕的对齐方向相同（若设置了 \an 或 \a，则按照 \an 或 \a 的值判断，否则按照样式的对齐方向判断）
        4. 满足下列条件之一：
            a. 两条字幕都既没有 \pos，也没有 \move 指令
            b. 两条字幕都有 \pos 指令，且两条字幕的 \pos 指令的参数相同
            （注：若两条字幕均有 \move 指令，或其中一条有 \move 指令，就算 \move 的参数一致，也不算冲突）

        通过以上内容可推导得出：
        1. 如果一条字幕拥有 \move 指令，那么一定不与其它字幕冲突
        2. 如果一条字幕为注释，且特效不为 [Kk]araoke，那么一定不与其它字幕冲突
        3. 如果一条字幕为对话，且特效为 fx，那么一定不与其它字幕冲突

    */

    auto begin = std::chrono::high_resolution_clock::now();

    if (type != AssFile::COMMIT_NEW && (type & WARNING_COMMIT_MASK) == 0)
    {
        // 没有我关心的变化
        return;
    }

    if (line == nullptr)
    {
        // Clear start-end cache map
        dialogueIndexCache.clear();

        // Clear cached eventProps
        eventProps.clear();
        eventProps.reserve(context->ass->Events.size());

        for (const AssDialogue &event : context->ass->Events)
        {
            if (!shouldEventBeConsidered(event))
            {
                continue;
            }

            DialogueProperties prop = parseEvent(event);

            if (!prop.hasMove)
            {
                eventProps.push_back(prop);
            }
        }
    }
    else
    {
        auto iter = dialogueIndexCache.find(line);
        bool considered = shouldEventBeConsidered(*line);
        DialogueProperties prop;

        if (considered)
        {
            prop = parseEvent(*line);
            if (prop.hasMove)
            {
                considered = false;
            }
        }

        if (iter == dialogueIndexCache.end() && considered)
        {
            eventProps.push_back(prop);
        }
        else if (iter != dialogueIndexCache.end())
        {
            size_t index = iter->second;
            eventProps[index] = prop;
            eventProps[index].considered = considered;
        }
    }

    // (Considered, Comment/Dialogue, Effect, Actor, Align(numpad), HasMove, PosX, PosY, start time, end time)
    std::sort(eventProps.begin(), eventProps.end(), [](const DialogueProperties &a, const DialogueProperties &b)
              {
        if (a.considered != b.considered) {
            return (int)(a.considered) < (int)(b.considered);
        }

        if (a.original->Comment != b.original->Comment) {
            return (int)(a.original->Comment) < (int)(b.original->Comment);
        }

        if (a.original->Effect != b.original->Effect) {
            return a.original->Effect < b.original->Effect;
        }

        if (a.original->Actor != b.original->Actor) {
            return a.original->Actor < b.original->Actor;
        }

        if (a.align != b.align) {
            return a.align < b.align;
        }

        if (a.hasMove != b.hasMove) {
            return (int)(a.hasMove) < (int)(b.hasMove);
        }

        if (a.posX != b.posX) {
            return a.posX < b.posX;
        }

        if (a.posY != b.posY) {
            return a.posY < b.posY;
        }

        if (a.original->Start == b.original->Start) {
            return a.original->Start < b.original->Start;
        }

        return a.original->End < b.original->End; });

    int last_warning = this->warning_num;
    this->warning_num = 0;

    // Check for overlapping subtitles
    for (size_t i = 0; i < eventProps.size(); i++)
    {
        dialogueIndexCache[eventProps[i].original] = i;

        // Only store AssDialogue* to dialogueIndexCache if i == 0
        if (i == 0)
        {
            continue;
        }

        DialogueProperties &a = eventProps[i - 1];
        DialogueProperties &b = eventProps[i];

        bool cond1 = (a.considered && b.considered);
        bool cond2 = (a.original->Style == b.original->Style && a.original->Style != "Default") ||
                     (a.original->Style == "Default" && b.original->Style == "Default" && a.original->Actor == b.original->Actor);
        bool cond3 = (a.align == b.align);
        bool cond4 = (std::fabs(a.posX - b.posX) <= 0.01 && std::fabs(a.posY - b.posY) <= 0.01);

        if (cond1 && cond2 && cond3 && cond4)
        {
            this->warning_num++;

            if (i == 0)
            {
                this->warning_num++;
            }
        }
    }

    if (this->warning_num != last_warning)
    {
        AnnounceGridWarningChanged(this->warning_num);
    }

    auto end = std::chrono::high_resolution_clock::now();
    wxMessageBox(
        wxString::Format(
            wxString(L"GridWarningController：%s更新，耗时 %lld ms"),
            line == nullptr ? wxString(L"全量") : wxString(L"增量"),
            std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()
        ),
        "DEBUG",
        wxOK | wxICON_INFORMATION,
        context->parent
    );
}

int GridWarningController::GetWarningNum()
{
    return this->warning_num;
}

std::vector<const AssDialogue *> GridWarningController::GetOverlappedEvents()
{
    return std::vector<const AssDialogue *>();
}
