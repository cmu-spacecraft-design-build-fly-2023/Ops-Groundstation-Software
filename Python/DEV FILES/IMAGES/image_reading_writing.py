import os

image_size = os.path.getsize('tinyimage.jpg')
image_message_count = int(image_size / 128)

if ((image_size % 128) > 0):
    image_message_count += 1

print("Image size is",image_size,"bytes")
print("This image requires",image_message_count,"messages")

bytes_remaining = image_size
sequence_counter = 0

image_array = []

send_bytes = open('tinyimage.jpg','rb')
rec_bytes = open('rximage.jpg','wb')

while (sequence_counter < image_message_count):
    if (bytes_remaining >= 128):
        payload = send_bytes.read(128)
    else:
        payload = send_bytes.read(bytes_remaining)
        
    bytes_remaining -= 128
    sequence_counter += 1

    image_array.append(payload)

for i in range(image_message_count):
    rec_bytes.write(image_array[i])

send_bytes.close()
rec_bytes.close()