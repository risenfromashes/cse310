#pragma once

class LocationInfo {
public:
  LocationInfo(int start) : start_line_(start), end_line_(start) {}
  LocationInfo(int start, int end) : start_line_(start), end_line_(end) {}

private:
  int start_line_;
  int end_line_;
};
