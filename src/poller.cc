#include "poller.h"

#include "channel.h"

bool Poller::HasChannel(Channel* channel) const {
  auto it = channel_maps_.find(channel->Fd());
  return it != channel_maps_.end() && it->second == channel;
}
