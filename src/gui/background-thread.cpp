/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2023, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "background-thread.h"

#include "gui/graphics/entity-graphics.h"
#include "gui/windows/message-box.h"
#include "models/common/u8strings.h"
#include "models/project/project.h"
#include "models/project/resource-compiler.h"
#include <functional>
#include <stop_token>

namespace UnTech::Gui {

static void markResourcesUnchanged(BackgroundThread::ChangesQueue& queue, Project::CompilerStatus& status, const Project::ProjectFile& pf)
{
    if (queue.resourceListMovedOrResized) {
        status.updateListSizeAndNames(pf);
    }
    else {
        for (auto r : queue.resources) {
            status.markUnchecked(r.type, r.index, pf);
        }
    }
    queue.resources.clear();
    queue.resourceListMovedOrResized = false;
}

static void bgThread(
    std::stop_token stopToken, // NOLINT(performance-unnecessary-value-param)
    mutex<BackgroundThread::ChangesQueue>& queue, std::binary_semaphore& queueChanged,
    const ProjectFileMutex& projectFile, Project::ProjectData& projectData, Project::CompilerStatus& compilerStatus)
{
    // Code to execute on `std::jthread::request_stop()`
    const auto stop_callback = std::stop_callback{
        stopToken, [&]() {
            // Unpause the background thread if it is waiting for the queue to change.
            queueChanged.release();
        }
    };

    try {
        while (!stopToken.stop_requested()) {
            // Wait until the queue has changed
            queueChanged.acquire();

            projectFile.read([&](const auto& pf) {
                // Process the queue
                queue.access([&](auto& queue) {
                    markResourcesUnchanged(queue, compilerStatus, pf);
                });

                Project::compileResources(compilerStatus, projectData, pf);

                const auto entityRomDataCompileId = compilerStatus.getCompileId(ProjectSettingsIndex::EntityRomData);
                processEntityGraphics(pf, projectData, entityRomDataCompileId);
            });
        }
    }
    catch (const std::exception& ex) {
        MsgBox::showMessage(u8"An exception occurred when compiling a resource",
                            stringBuilder(convert_old_string(ex.what()),
                                          u8"\n\n\nThis should not happen."
                                          u8"\n\nThe resource compiler is now disabled."));

        compilerStatus.markAllUnchecked();
    }
}

BackgroundThread::BackgroundThread(const ProjectFileMutex& pf, Project::ProjectData& data, Project::CompilerStatus& status)
    : queue()
    , queueChanged(0)
    , projectFile(pf)
    , projectData(data)
    , compilerStatus(status)
{
    thread = std::jthread(bgThread,
                          std::ref(queue), std::ref(queueChanged),
                          std::cref(projectFile), std::ref(projectData), std::ref(compilerStatus));

    markResourceListMovedOrResized();
}

void BackgroundThread::markResourceUnchecked(ItemIndex index)
{
    queue.access([&](auto& q) {
        q.resources.push_back(index);
    });
    queueChanged.release();
}

void BackgroundThread::markResourceListMovedOrResized()
{
    queue.access([&](auto& q) {
        q.resourceListMovedOrResized = true;
    });
    queueChanged.release();
}

}
