#include <iostream>
#include <fstream>
#include <algorithm>
#include <cxxopts/cxxopts.hpp>
#include <json/json.h>

#define TODO "Todo"
#define DONE "Done"

enum TaskState { kDone, kTodo, kInvalid };

TaskState FromName(std::string name) {
  if (name == TODO) {
    return kTodo;
  }
  else if (name == DONE) {
    return kDone;
  }
  else {
    return kInvalid;
  }
}

std::string FromState(TaskState state) {
  switch (state) {
    case kDone: return DONE;
    case kTodo: return TODO;
    default: return "Invalid";
  }
}

void Print(const std::string& message) {
  std::cout << message << std::endl;
}

const std::string Join(const std::vector<std::string> strings,
                        const std::string separator) {
  std::string result;
  for (auto& s : strings) {
    result += result.length() ? separator + s : s;
  }
  return result;
}

struct Task {
  Task(const int id, const std::string text, const TaskState state)
    : id_(id), text_(text), state_(state) {
  }

  std::string text() {
    return text_;
  }

  int id() {
    return id_;
  }

  TaskState state() {
    return state_;
  }

  const Task* Done() {
    return new Task{id_, text_, kDone};
  }

  const Task* Undo() {
    return new Task{id_, text_, kTodo};
  }

  std::string ToString() {
    return "[" + std::to_string(id_) + "] " + text_  + " - " + FromState(state_);
  }

  Json::Value Marshal() {
    Json::Value value;
    value["id"] = id_;
    value["text"] = text_;
    value["state"] = FromState(state_);
    return value;
  }

private:
  const int id_;
  const std::string text_;
  const TaskState state_;
};

struct TaskManager {

  TaskManager(std::string& path) : filepath_(path) {
    if (!hasTasksFile()) {
      std::ofstream outstream{filepath_};
      outstream << "{ \"tasks\" : [] }" << std::endl;
      outstream.close();
    }
    std::ifstream file{filepath_, std::ifstream::binary};
    file >> root_;
    file.close();
    const Json::Value tasks = root_["tasks"];
    for (int i = 0; i < tasks.size(); i++) {
      const Json::Value task = tasks[i];
      int currentId{task["id"].asInt()};
      nextId_ = std::max(nextId_, currentId);
      tasks_.emplace(
        currentId,
        Task{currentId, task["text"].asString(), FromName(task["state"].asString())});
    }
    nextId_++;
  }

  void List() {
    if (tasks_.size() || newTasks_.size()) {
      Print("Current tasks:");
    }
    else {
      Print("No tasks on this list!");
    }
    for (auto& entry : tasks_) {
      Task& task = entry.second;
      Print(task.ToString());
    }
    for (auto& entry : newTasks_) {
      Task& task = entry.second;
      Print(task.ToString());
    }
  }

  void Add(const std::string& task) {
    newTasks_.emplace(nextId_, Task{nextId_, task, kTodo});
    nextId_++;
  }

  void Remove(const int id) {
    auto it = tasks_.find(id);
    if (it != tasks_.end()) {
      tasks_.erase(it);
    }
    else {
      Print("Task " + std::to_string(id) + " not found!");
    }
  }

  void Finish(const int id) {
    auto it = tasks_.find(id);
    if (it != tasks_.end()) {
      const Task& task = *it->second.Done();
      tasks_.erase(it);
      tasks_.emplace(id, task);
    }
    else {
      Print("Task " + std::to_string(id) + " not found!");
    }
  }

  void Undo(const int id) {
    auto it = tasks_.find(id);
    if (it != tasks_.end()) {
      const Task& task = *it->second.Undo();
      tasks_.erase(it);
      tasks_.emplace(id, task);
    }
    else {
      Print("Task " + std::to_string(id) + " not found!");
    }
  }

  void Persist() {
    Json::StyledWriter writer;
    Json::Value tasks = root_["tasks"];
    tasks.clear();
    for (auto& entry : tasks_) {
      tasks.append(entry.second.Marshal());
    }
    for (auto& entry : newTasks_) {
      tasks.append(entry.second.Marshal());
    }
    root_["tasks"] = tasks;
    // store in human friendly format
    
    std::string outputConfig = writer.write(root_);
    std::ofstream outstream (filepath_, std::ofstream::out);
    outstream << outputConfig;
    outstream.close();
    
  }

private:
  Json::Value root_;
  std::map<int, Task> tasks_;
  std::map<int, Task> newTasks_;
  int nextId_{0};
  std::string& filepath_;

  bool hasTasksFile() {
    std::ifstream file_test{filepath_, std::ifstream::binary};
    return file_test.good();
  }
};

struct TaskCommand {
  TaskCommand(const std::string& filepath, const std::string& text,
              const int finishId, const int undoId, const int removeId)
    : filepath_(filepath), text_(text), finishId_(finishId),
      undoId_(undoId), removeId_(removeId) {
  }

  std::string text() {
    return text_;
  }

  bool HasText() {
    return text_.length();
  }

  int finishId() {
    return finishId_;
  }

  int undoId() {
    return undoId_;
  }

  int removeId() {
    return removeId_;
  }

  std::string filepath() {
    return filepath_;
  }

private:
  const std::string filepath_;
  const std::string text_;
  const int finishId_;
  const int undoId_;
  const int removeId_;
};

std::shared_ptr<TaskCommand> ParseOptions(int argc, char* argv[]) {
  try {
    cxxopts::Options options(argv[0], " - example command line options");

    options.add_options()
      ("text", "", cxxopts::value<std::vector<std::string>>())
      ("file", "Tasks file.", cxxopts::value<std::string>())
      ("f,finish", "Marks the task as Done.", cxxopts::value<int>())
      ("u,undo", "Marks the task as Todo.", cxxopts::value<int>())
      ("r,remove", "Removes the given task.", cxxopts::value<int>())
      ("help", "Print help");

    options.parse_positional("text");
    options.parse(argc, argv);

    if (options.count("help")) {
      std::cout << options.help({""}) << std::endl;
      exit(0);
    }

    std::string text;
    if (options.count("text")) {
      std::vector<std::string> allArgs{options["text"].as<std::vector<std::string>>()};
      text = Join(allArgs, " ");
    }

    int finishId{0};
    if (options.count("finish")) {
      finishId = options["finish"].as<int>();
    }

    int undoId{0};
    if (options.count("undo")) {
      undoId = options["undo"].as<int>();
    }

    int removeId{0};
    if (options.count("remove")) {
      removeId = options["remove"].as<int>();
    }

    std::string filepath;
    if (options.count("file")) {
      filepath = options["file"].as<std::string>();
    }
    else {
      std::cerr << "No tasks file was specified!" << std::endl;
      exit(1);
    }
    ;
    return std::shared_ptr<TaskCommand>{
      new TaskCommand{filepath, text, finishId, undoId, removeId}};
  }
  catch (const cxxopts::OptionException& e) {
    std::cout << "error parsing options: " << e.what() << std::endl;
    exit(1);
  }
}

int main(int argc, char* argv[]) {
  std::shared_ptr<TaskCommand> options_ptr(ParseOptions(argc, argv));
  TaskCommand command = *(options_ptr.get());

  std::string path(command.filepath());
  TaskManager manager(path);
  bool needsPersist{false};
  if (command.HasText()) {
    manager.Add(command.text());
    needsPersist = true;
  }
  if (command.finishId()) {
    manager.Finish(command.finishId());
    needsPersist = true;
  }
  if (command.undoId()) {
    manager.Undo(command.undoId());
    needsPersist = true;
  }
  if (command.removeId()) {
    manager.Remove(command.removeId());
    needsPersist = true;
  }
  if (needsPersist) {
    manager.Persist();
  }
  manager.List();

  return 0;
}
