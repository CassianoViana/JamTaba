#include "LocalInputGroup.h"
#include "LocalInputNode.h"

using namespace Audio;

LocalInputGroup::LocalInputGroup(int groupIndex, Audio::LocalInputNode *firstInput) :
    groupIndex(groupIndex),
    transmiting(true)
{
    addInput(firstInput);
}

LocalInputGroup::~LocalInputGroup()
{
    groupedInputs.clear();
}

void LocalInputGroup::addInput(Audio::LocalInputNode *input)
{
    groupedInputs.append(input);
}

void LocalInputGroup::mixGroupedInputs(Audio::SamplesBuffer &out)
{
    foreach (Audio::LocalInputNode *inputTrack, groupedInputs) {
        if (!inputTrack->isMuted())
            out.add(inputTrack->getLastBuffer());
    }
}

void LocalInputGroup::removeInput(Audio::LocalInputNode *input)
{
    if (!groupedInputs.removeOne(input))
        qCritical() << "the input track was not removed!";
}

int LocalInputGroup::getMaxInputChannelsForEncoding() const
{
    if (groupedInputs.size() > 1)
        return 2;    // stereo encoding
    if (!groupedInputs.isEmpty()) {
        if (groupedInputs.first()->isMidi())
            return 2;    // just one midi track, use stereo encoding
        if (groupedInputs.first()->isAudio())
            return groupedInputs.first()->getAudioInputRange().getChannels();
        if (groupedInputs.first()->isNoInput())
            return 2;    // allow channels using noInput but processing some vst looper in stereo
    }
    return 0;    // no channels to encoding
}

void LocalInputGroup::setTransmitingStatus(bool transmiting)
{
    this->transmiting = transmiting;
}
