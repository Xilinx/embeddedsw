Scenechange IP calculates sum of absolute differences (SAD) between
consecutive video frames received on either memory interface or stream
interface. Interface selection can be done from the GUI.

Scenechange IP driver gets SAD values from IP, compares and notifies
the user about scene change.

Scenechange IP has two applications for testing.
1. Memory Mode.
2. Stream Mode.

Procedure to test SceneChange IP in Memory mode:
------------------------------------------------
Scenechange IP supports 8 streams in the memory mode. User has to
configure the number of streams, height, width, stride, buffer
address, threshold value for SAD and subsample value into the
registers.

Final scene change determination will be made based on threshold
value (i.e 0.5 - 1) specified by the user. Refer scene change PG
for threshold calculation.

Application is trying to write different patterns into the buffer
address with the some delay and waiting for scenechange detection.

SceneChange driver notifies the SAD with stream number(on which
stream scene change has happend) with the help of callback
function.

Procedure to test Scenechange IP in Stream mode:
------------------------------------------------

Scenechange IP supports only one stream in the stream mode. User has
to configure the height, width, stride, buffer address, threshold
value for SAD and subsample value into the registers.

Final scene change determination will be made based on threshold value
specified by the user. Refer scene change PG for threshold calculation.

Application is trying to generate different TPG patterns using TPG with
the delay of 2sec and observing scenechange happens are not based on
the threshold value.
