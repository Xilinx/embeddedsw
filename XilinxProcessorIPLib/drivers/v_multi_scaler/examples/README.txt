When executed on the board the example application will select each use case from the
use case list and sets the input and output configuration of the multiscaler IP accordingly.
The source buffers are filled by the application with some fixed pattern. The application
waits for the interrupt from the IP and prints the contents of the destination buffers to
validate if the scaling is performed.

