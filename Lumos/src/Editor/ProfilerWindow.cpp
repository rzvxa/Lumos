#include "lmpch.h"
#include "ProfilerWindow.h"
#include "App/Engine.h"

#include <IconFontCppHeaders/IconsFontAwesome5.h>

//Based on https://github.com/Raikiri/LegitProfiler

namespace Lumos
{
#define RGBA_LE(col) (((col & 0xff000000) >> (3 * 8)) + ((col & 0x00ff0000) >> (1 * 8)) + ((col & 0x0000ff00) << (1 * 8)) + ((col & 0x000000ff) << (3 * 8)))
	uint32_t turqoise = RGBA_LE(0x1abc9cffu);
	uint32_t greenSea = RGBA_LE(0x16a085ffu);

	uint32_t emerald = RGBA_LE(0x2ecc71ffu);
	uint32_t nephritis = RGBA_LE(0x27ae60ffu);

	uint32_t peterRiver = RGBA_LE(0x3498dbffu); //blue
	uint32_t belizeHole = RGBA_LE(0x2980b9ffu);

	uint32_t amethyst = RGBA_LE(0x9b59b6ffu);
	uint32_t wisteria = RGBA_LE(0x8e44adffu);

	uint32_t sunFlower = RGBA_LE(0xf1c40fffu);
	uint32_t orange = RGBA_LE(0xf39c12ffu);

	uint32_t carrot = RGBA_LE(0xe67e22ffu);
	uint32_t pumpkin = RGBA_LE(0xd35400ffu);

	uint32_t alizarin = RGBA_LE(0xe74c3cffu);
	uint32_t pomegranate = RGBA_LE(0xc0392bffu);

	uint32_t clouds = RGBA_LE(0xecf0f1ffu);
	uint32_t silver = RGBA_LE(0xbdc3c7ffu);
	uint32_t imguiText = RGBA_LE(0xF2F5FAFFu);

	uint32_t colour[16] = { turqoise , greenSea,emerald, nephritis , peterRiver , belizeHole , amethyst ,wisteria ,sunFlower ,orange , carrot , pumpkin , alizarin, pomegranate,clouds, silver };

	ProfilerWindow::ProfilerWindow()
	{
		m_Name = ICON_FA_STOPWATCH" Profiler###profiler";
		m_SimpleName = "Profiler";

		m_UpdateTimer = 0.0f;
		m_UpdateFrequency = 0.1f;

		m_CPUGraph = ProfilerGraph();

		stopProfiling = false;
		frameOffset = 0;
		frameWidth = 3;
		frameSpacing = 1;
		useColoredLegendText = true;
		prevFpsFrameTime = std::chrono::system_clock::now();
		fpsFramesCount = 0;
		avgFrameTime = 1.0f;
	}

	void ProfilerWindow::OnImGui()
	{
		ImGui::Begin(m_Name.c_str(), 0, ImGuiWindowFlags_NoScrollbar);

		auto profiler = Profiler::Instance();

		m_CPUGraph.Update();

		ImVec2 canvasSize = ImGui::GetContentRegionAvail();

		int sizeMargin = int(ImGui::GetStyle().ItemSpacing.y);
		int maxGraphHeight = 600;
		int availableGraphHeight = (int(canvasSize.y) - sizeMargin) / 2;
		int graphHeight = std::min(maxGraphHeight, availableGraphHeight);
		int legendWidth = 350;
		int graphWidth = int(canvasSize.x) - legendWidth;
		m_CPUGraph.RenderTimings(graphWidth, legendWidth, graphHeight, frameOffset);
		if (graphHeight * 2 + sizeMargin + sizeMargin < canvasSize.y)
		{
			ImGui::Columns(2);
			size_t textSize = 50;
			ImGui::Checkbox("Stop profiling", &profiler->IsEnabled());
			//ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - textSize);
			ImGui::Checkbox("Colored legend text", &useColoredLegendText);
			ImGui::DragInt("Frame offset", &frameOffset, 1.0f, 0, 400);
			ImGui::NextColumn();

			ImGui::SliderInt("Frame width", &frameWidth, 1, 4);
			ImGui::SliderInt("Frame spacing", &frameSpacing, 0, 2);
			ImGui::SliderFloat("Transparency", &ImGui::GetStyle().Colors[ImGuiCol_WindowBg].w, 0.0f, 1.0f);
			ImGui::Columns(1);
		}
		if (!profiler->IsEnabled())
			frameOffset = 0;
		m_CPUGraph.frameWidth = frameWidth;
		m_CPUGraph.frameSpacing = frameSpacing;
		m_CPUGraph.useColoredLegendText = useColoredLegendText;

		ImGui::End();

	}

	ProfilerGraph::ProfilerGraph()
	{
		m_Reports.reserve(300);
		currFrameIndex = 0;
	}

	void ProfilerGraph::Update()
	{
		auto profiler = Profiler::Instance();
		if (profiler->IsEnabled())
		{
			auto report = profiler->GenerateReport();
			profiler->ClearHistory();

			m_Reports.emplace_back(report);
		}

		auto &currFrame = m_Reports.back();
	
		currFrame.taskStatsIndex.resize(currFrame.actions.size());

		for (size_t taskIndex = 0; taskIndex < currFrame.actions.size(); taskIndex++)
		{
			auto &task = currFrame.actions[taskIndex];
			auto it = taskNameToStatsIndex.find(task.name);
			if (it == taskNameToStatsIndex.end())
			{
				taskNameToStatsIndex[task.name] = taskStats.size();
				TaskStats taskStat;
				taskStat.maxTime = -1.0;
				taskStats.push_back(taskStat);
			}
			currFrame.taskStatsIndex[taskIndex] = taskNameToStatsIndex[task.name];
		}
		currFrameIndex = (currFrameIndex + 1) % m_Reports.size();

		RebuildTaskStats(currFrameIndex, 300/*frames.size()*/);
	}

	inline void ProfilerGraph::Rect(ImDrawList * drawList, const Maths::Vector2 & minPoint, const Maths::Vector2 & maxPoint, uint32_t col, bool filled)
	{
		if (filled)
			drawList->AddRectFilled(ImVec2(minPoint.x, minPoint.y), ImVec2(maxPoint.x, maxPoint.y), col);
		else
			drawList->AddRect(ImVec2(minPoint.x, minPoint.y), ImVec2(maxPoint.x, maxPoint.y), col);
	}
	inline void ProfilerGraph::Text(ImDrawList * drawList, const Maths::Vector2 & point, uint32_t col, const char * text)
	{
		drawList->AddText(ImVec2(point.x, point.y), col, text);
	}
	inline void ProfilerGraph::Triangle(ImDrawList * drawList, const std::array<Maths::Vector2, 3>& points, uint32_t col, bool filled)
	{
		if (filled)
			drawList->AddTriangleFilled(ImVec2(points[0].x, points[0].y), ImVec2(points[1].x, points[1].y), ImVec2(points[2].x, points[2].y), col);
		else
			drawList->AddTriangle(ImVec2(points[0].x, points[0].y), ImVec2(points[1].x, points[1].y), ImVec2(points[2].x, points[2].y), col);
	}
	inline void ProfilerGraph::RenderTaskMarker(ImDrawList * drawList, const Maths::Vector2 & leftMinPoint, const Maths::Vector2 & leftMaxPoint, const Maths::Vector2 & rightMinPoint, const Maths::Vector2 & rightMaxPoint, uint32_t col)
	{
		Rect(drawList, leftMinPoint, leftMaxPoint, col, true);
		Rect(drawList, rightMinPoint, rightMaxPoint, col, true);
		std::array<ImVec2, 4> points = {
			ImVec2(leftMaxPoint.x, leftMinPoint.y),
			ImVec2(leftMaxPoint.x, leftMaxPoint.y),
			ImVec2(rightMinPoint.x, rightMaxPoint.y),
			ImVec2(rightMinPoint.x, rightMinPoint.y)
		};
		drawList->AddConvexPolyFilled(points.data(), int(points.size()), col);
	}
	inline void ProfilerGraph::RenderGraph(ImDrawList * drawList, const Maths::Vector2 & graphPos, const Maths::Vector2 & graphSize, size_t frameIndexOffset)
	{
		Rect(drawList, graphPos, graphPos + graphSize, 0xffffffff, false);
		float maxFrameTime = 1.0f / 30.0f;
		float heightThreshold = 1.0f;

		

		for (size_t frameNumber = 0; frameNumber < m_Reports.size(); frameNumber++)
		{
			size_t frameIndex = (currFrameIndex - frameIndexOffset - 1 - frameNumber + 2 * m_Reports.size()) % m_Reports.size();

			Maths::Vector2 framePos = graphPos + Maths::Vector2(graphSize.x - 1 - frameWidth - (frameWidth + frameSpacing) * frameNumber, graphSize.y - 1);
			if (framePos.x < graphPos.x + 1)
				break;
			Maths::Vector2 taskPos = framePos;// +Maths::Vector2(0.0f, 0.0f);
			auto &frame = m_Reports[frameIndex];
			int taskIndex = 0;
			float currentTime = 0.0f;

			for (auto task : frame.actions)
			{
				/*	float taskStartHeight = (float(task.startTime) / maxFrameTime) * graphSize.y;
					float taskEndHeight = (float(task.endTime) / maxFrameTime) * graphSize.y;*/

				float duration = (float(task.duration)/* / maxFrameTime*/) * graphSize.y / 50.0f;

				//float taskEndHeight = (float(task.endTime) / maxFrameTime) * graphSize.y;

				if (abs(duration) > heightThreshold)
				{
					Rect(drawList, taskPos + Maths::Vector2(0.0f, -currentTime), taskPos + Maths::Vector2(frameWidth, -duration), colour[taskIndex++ % 16], true);
					currentTime += duration;
				}

				//float taskStartHeight = (float(task.startTime) / maxFrameTime) * graphSize.y;
				//float taskEndHeight = (float(task.endTime) / maxFrameTime) * graphSize.y;
				////taskMaxCosts[task.name] = std::max(taskMaxCosts[task.name], task.endTime - task.startTime);
				//if (abs(taskEndHeight - taskStartHeight) > heightThreshold)
				//	Rect(drawList, taskPos + glm::vec2(0.0f, -taskStartHeight), taskPos + glm::vec2(frameWidth, -taskEndHeight), task.color, true);
			}
		}
	}
	inline void ProfilerGraph::RenderTimings(int graphWidth, int legendWidth, int height, int frameIndexOffset)
	{
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		const Maths::Vector2 widgetPos = Maths::Vector2(ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y);
		RenderGraph(drawList, widgetPos, Maths::Vector2(graphWidth, height), frameIndexOffset);
		RenderLegend(drawList, widgetPos + Maths::Vector2(graphWidth, 0.0f), Maths::Vector2(legendWidth, height), frameIndexOffset);
		ImGui::Dummy(ImVec2(float(graphWidth + legendWidth), float(height)));
	}

	void ProfilerGraph::RenderLegend(ImDrawList *drawList, const Maths::Vector2& legendPos, const Maths::Vector2& legendSize, size_t frameIndexOffset)
	{
		float markerLeftRectMargin = 3.0f;
		float markerLeftRectWidth = 5.0f;
		float maxFrameTime = 1.0f / 30.0f;
		float markerMidWidth = 30.0f;
		float markerRightRectWidth = 10.0f;
		float markerRigthRectMargin = 3.0f;
		float markerRightRectHeight = 10.0f;
		float markerRightRectSpacing = 4.0f;
		float nameOffset = 30.0f;
		Maths::Vector2 textMargin = Maths::Vector2(5.0f, -3.0f);

		auto &currFrame = m_Reports[(currFrameIndex - frameIndexOffset - 1 + 2 * m_Reports.size()) % m_Reports.size()];
		size_t maxTasksCount = size_t(legendSize.y / (markerRightRectHeight + markerRightRectSpacing));

		for (auto &taskStat : taskStats)
		{
			taskStat.onScreenIndex = size_t(-1);
		}

		size_t tasksToShow = std::min<size_t>(taskStats.size(), maxTasksCount);
		size_t tasksShownCount = 0;

		float currentTime = 0.0f;
		for (size_t taskIndex = 0; taskIndex < currFrame.actions.size(); taskIndex++)
		{
			auto &task = currFrame.actions[taskIndex];
			auto &stat = taskStats[currFrame.taskStatsIndex[taskIndex]];

			if (stat.priorityOrder >= tasksToShow)
				continue;

			if (stat.onScreenIndex == size_t(-1))
			{
				stat.onScreenIndex = tasksShownCount++;
			}
			else
				continue;
			float taskStartHeight = (float(task.duration) / maxFrameTime) * legendSize.y;
			float taskEndHeight = (float(task.duration) / maxFrameTime) * legendSize.y;

			float duration = (float(task.duration)/* / maxFrameTime*/) * legendSize.y / 50.0f;

			Maths::Vector2 markerLeftRectMin = legendPos + Maths::Vector2(markerLeftRectMargin, legendSize.y);
			Maths::Vector2 markerLeftRectMax = markerLeftRectMin + Maths::Vector2(markerLeftRectWidth, 0.0f);
			markerLeftRectMin.y -= currentTime;// taskStartHeight;
			markerLeftRectMax.y -= duration;// taskEndHeight;

			currentTime += duration;

			Maths::Vector2 markerRightRectMin = legendPos + Maths::Vector2(markerLeftRectMargin + markerLeftRectWidth + markerMidWidth, legendSize.y - markerRigthRectMargin - (markerRightRectHeight + markerRightRectSpacing) * stat.onScreenIndex);
			Maths::Vector2 markerRightRectMax = markerRightRectMin + Maths::Vector2(markerRightRectWidth, -markerRightRectHeight);
			RenderTaskMarker(drawList, markerLeftRectMin, markerLeftRectMax, markerRightRectMin, markerRightRectMax, colour[taskIndex % 16]);

			uint32_t textColor = useColoredLegendText ? colour[taskIndex % 16] : imguiText;// task.color;

			float taskTimeMs = float(task.duration);
			std::ostringstream timeText;
			timeText.precision(2);
			timeText << std::fixed << std::string("[") << (taskTimeMs);

			Text(drawList, markerRightRectMax + textMargin, textColor, timeText.str().c_str());
			Text(drawList, markerRightRectMax + textMargin + Maths::Vector2(nameOffset, 0.0f), textColor, (std::string("    ms] ") + task.name).c_str());
		}
	}

	void ProfilerGraph::RebuildTaskStats(size_t endFrame, size_t framesCount)
	{
		for (auto &taskStat : taskStats)
		{
			taskStat.maxTime = -1.0f;
			taskStat.priorityOrder = size_t(-1);
			taskStat.onScreenIndex = size_t(-1);
		}

		for (size_t frameNumber = 0; frameNumber < framesCount; frameNumber++)
		{
			size_t frameIndex = (endFrame - 1 - frameNumber + m_Reports.size()) % m_Reports.size();
			auto &frame = m_Reports[frameIndex];
			for (size_t taskIndex = 0; taskIndex < frame.actions.size(); taskIndex++)
			{
				auto &task = frame.actions[taskIndex];
				auto &stats = taskStats[frame.taskStatsIndex[taskIndex]];
				stats.maxTime = std::max(stats.maxTime, task.duration);
			}
		}
		std::vector<size_t> statPriorities;
		statPriorities.resize(taskStats.size());
		for (size_t statIndex = 0; statIndex < taskStats.size(); statIndex++)
			statPriorities[statIndex] = statIndex;

		std::sort(statPriorities.begin(), statPriorities.end(), [this](size_t left, size_t right) {return taskStats[left].maxTime > taskStats[right].maxTime; });
		for (size_t statNumber = 0; statNumber < taskStats.size(); statNumber++)
		{
			size_t statIndex = statPriorities[statNumber];
			taskStats[statIndex].priorityOrder = statNumber;
		}
	}
}
//if (Profiler::Instance()->IsEnabled())
//{
//	m_UpdateTimer += Engine::Instance()->GetTimeStep()->GetElapsedMillis();
//}
//
//ImGui::Begin(m_Name.c_str());
//{
//	auto profiler = Profiler::Instance();
//	ImGui::Checkbox("Profiler Enabled", &profiler->IsEnabled());
//	ImGui::InputFloat("Update Frequency", &m_UpdateFrequency);
//	ImGui::Text("Report period duration : %f ms", m_Report.elaspedTime);
//	ImGui::Text("Threads : %i", m_Report.workingThreads);
//	ImGui::Text("Frames : %i", m_Report.elapsedFrames);
//
//	if (profiler->IsEnabled())
//	{
//		if (m_UpdateTimer >= m_UpdateFrequency)
//		{
//			m_Report = profiler->GenerateReport();
//			profiler->ClearHistory();
//			m_UpdateTimer = 0.0f;
//		}
//
//		if (m_Report.actions.empty())
//		{
//			ImGui::Text("Collecting Data");
//		}
//		else
//		{
//			for (const auto& action : m_Report.actions)
//			{
//				ImVec4 colour;
//
//				if (action.percentage <= 25.0f)
//					colour = { 0.2f,0.8f,0.2f,1.0f };
//				else if (action.percentage <= 50.0f)
//					colour = { 0.2f,0.6f,0.6f,1.0f };
//				else if (action.percentage <= 75.0f)
//					colour = { 0.7f,0.2f,0.3f,1.0f };
//				else
//					colour = { 0.8f,0.2f,0.2f,1.0f };
//
//				ImGui::TextColored(colour, "%s %f ms | %f ms per call | %f percent | %llu calls", action.name.c_str(), action.duration, action.duration / action.calls, action.percentage, action.calls);
//			}
//		}
//	}
//	else
//	{
//		ImGui::Text("Profiler Disabled");
//	}
//}
//ImGui::End();