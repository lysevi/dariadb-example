#include <libdariadb/engines/engine.h>
#include <libdariadb/utils/fs.h>
#include <iostream>

class QuietLogger : public dariadb::utils::ILogger {
public:
  void message(dariadb::utils::LOG_MESSAGE_KIND kind, const std::string &msg) override {}
};

class Callback : public dariadb::IReadCallback {
public:
  Callback() {}

  void apply(const dariadb::Meas &measurement) override {
    std::cout << " id: " << measurement.id
              << " timepoint: " << dariadb::timeutil::to_string(measurement.time)
              << " value:" << measurement.value << std::endl;
  }

  void is_end() override {
    std::cout << "calback end." << std::endl;
    dariadb::IReadCallback::is_end();
  }
};

int main(int argc, char **argv) {
  const std::string storage_path = "exampledb";
  // remove old storage.
  if (dariadb::utils::fs::path_exists(storage_path)) {
    dariadb::utils::fs::rm(storage_path);
  }

  // reset standert logger
  dariadb::utils::ILogger_ptr log_ptr{new QuietLogger()};
  dariadb::utils::LogManager::start(log_ptr);

  // create defaul settings
  auto settings = dariadb::storage::Settings::create(storage_path);
  settings->save();

  auto storage = std::make_unique<dariadb::Engine>(settings);

  auto m = dariadb::Meas();
  auto start_time = dariadb::timeutil::current_time();

  m.time = start_time;
  for (size_t i = 0; i < 10; ++i) {
    if (i % 2) {
      m.id = dariadb::Id(0);
    } else {
      m.id = dariadb::Id(1);
    }

    m.time++;
    m.value++;
    m.flag = 100 + i % 2;
    auto status = storage->append(m);
    if (status.writed != 1) {
      std::cerr << "Error: " << dariadb::to_string(status.error) << std::endl;
    }
  }
  // array with writed id`s
  dariadb::IdArray all_id{dariadb::Id(0), dariadb::Id(1)};

  // query list of values in interval
  dariadb::QueryInterval qi(all_id, dariadb::Flag(), start_time, m.time);
  dariadb::MeasArray readed_values = storage->readInterval(qi);
  std::cout << "Readed: " << readed_values.size() << std::endl;
  for (auto measurement : readed_values) {
    std::cout << " param: " << measurement.id
              << " timepoint: " << dariadb::timeutil::to_string(measurement.time)
              << " value:" << measurement.value << std::endl;
  }

  // query timepoint
  dariadb::QueryTimePoint qp(all_id, dariadb::Flag(), m.time);
  dariadb::Id2Meas timepoint = storage->readTimePoint(qp);
  std::cout << "Timepoint: " << std::endl;
  for (auto kv : timepoint) {
    auto measurement = kv.second;
    std::cout << " param: " << kv.first
              << " timepoint: " << dariadb::timeutil::to_string(measurement.time)
              << " value:" << measurement.value << std::endl;
  }

  // current values
  dariadb::Id2Meas cur_values = storage->currentValue(all_id, dariadb::Flag());
  std::cout << "Current: " << std::endl;
  for (auto kv : timepoint) {
    auto measurement = kv.second;
    std::cout << " id: " << kv.first
              << " timepoint: " << dariadb::timeutil::to_string(measurement.time)
              << " value:" << measurement.value << std::endl;
  }

  // apply callback to interval
  std::cout << "Callback in interval: " << std::endl;
  Callback callback;
  storage->foreach (qi, &callback);
  callback.wait();

  // apply callback to values in timepoint
  std::cout << "Callback in timepoint: " << std::endl;
  storage->foreach (qp, &callback);
  callback.wait();

  { // query statistic for Id==0 in interval
    auto stat = storage->stat(dariadb::Id(0), start_time, m.time);
    std::cout << "count: " << stat.count << std::endl;
    std::cout << "time: [" << dariadb::timeutil::to_string(stat.minTime) << " "
              << dariadb::timeutil::to_string(stat.maxTime) << "]" << std::endl;
    std::cout << "val: [" << stat.minValue << " " << stat.maxValue << "]" << std::endl;
    std::cout << "sum: " << stat.sum << std::endl;
  }
}
