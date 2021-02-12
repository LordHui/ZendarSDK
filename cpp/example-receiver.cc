#include "zendar-api.h"
#include "data.pb.h"
#include <unordered_map>

int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("Usage: example-receiver URI");
  }
  zendar::ZendarReceiver rcv(argv[1]);
  zendar::ZendarError error;

  error = rcv.SubscribeImages(100);
  if (error)
    printf("Failed to subscribe to images");
  error = rcv.SubscribeTracker(100);
  if (error)
    printf("Failed to subscribe to tracker");
  error = rcv.SubscribeTracklog(100);
  if (error)
    printf("Failed to subscribe to tracklog");
  error = rcv.SubscribeLogs(100);
  if (error)
    printf("Failed to subscribe to logs");

  printf("Successful Initialization\n");
  zen_proto::data::Image image;
  zen::tracker::message::TrackerState tracker_state;
  zen_proto::data::Position position;
  zen_proto::data::LogRecord log;

  std::unordered_map<std::string, std::uint64_t> last_imaging_frame_id;
  std::unordered_map<std::string, std::uint64_t> last_tracking_frame_id;
  while (true) {
    error = rcv.NextImage(image);
    if (error) {
      printf("Failed to receive image\n");
    } else {
      printf("Received image, timestamp: %f \n", image.timestamp());

      // checking for dropped Imaging data
      if ( last_imaging_frame_id.find(image.serial()) == last_imaging_frame_id.end()){
        last_imaging_frame_id[image.serial()] = image.frame_id();
      } else {
        if (image.frame_id() != last_imaging_frame_id[image.serial()]+1){
          printf("Imaging network DROP detected");
        }
        last_imaging_frame_id[image.serial()] = image.frame_id();
      }
    }
    error = rcv.NextTracker(tracker_state);
    if (error) {
      printf("Failed to receive tracker_state\n");
    } else {
      printf("Received tracker_state, timestamp: %f \n",
             tracker_state.timestamp().common());
      // checking for dropped Tracking data
      if ( last_tracking_frame_id.find(tracker_state.serial()) == last_tracking_frame_id.end()){
        last_tracking_frame_id[tracker_state.serial()] = tracker_state.frame_id();
      } else {
        if (tracker_state.frame_id() != last_tracking_frame_id[tracker_state.serial()]+1){
          printf("Tracking network DROP detected");
        }
        last_tracking_frame_id[tracker_state.serial()] = tracker_state.frame_id();
      }

    }
    error = rcv.NextTracklog(position);
    if (error) {
      printf("Failed to receive position\n");
    } else {
      printf("Received position:\n %f\n %f\n %f \n",
             position.position().x(),
             position.position().y(),
             position.position().z());
    }
    error = rcv.NextLogMessage(log);
    if (error) {
      printf("Failed to receive log\n");
    } else {
      printf("Received log: %s\n", log.message().c_str());
    }
  }
  error = rcv.UnsubscribeImages();
  if (error)
    printf("Failed to unsubscribe from images");
  error = rcv.UnsubscribeTracker();
  if (error)
    printf("Failed to unsubscribe from tracker");
  error = rcv.UnsubscribeTracklog();
  if (error)
    printf("Failed to subscribe from tracklog");
  error = rcv.UnsubscribeLogs();
  if (error)
    printf("Failed to subscribe from logs");

  return 1;
}
