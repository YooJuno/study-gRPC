#include <iostream>
#include "my_message.pb.h"

void PrintFieldNamesAndValues(const google::protobuf::Message& message) {
    const google::protobuf::Descriptor* descriptor = message.GetDescriptor();
    const google::protobuf::Reflection* reflection = message.GetReflection();
    
    for (int i = 0; i < descriptor->field_count(); ++i) {
        const google::protobuf::FieldDescriptor* field = descriptor->field(i);
        std::cout << "Field name: " << field->name() << std::endl;

        if (field->type() == google::protobuf::FieldDescriptor::TYPE_INT32) {
            int value = reflection->GetInt32(message, field);
            std::cout << "Value: " << value << std::endl;
        } else if (field->type() == google::protobuf::FieldDescriptor::TYPE_STRING) {
            std::string value = reflection->GetString(message, field);
            std::cout << "Value: " << value << std::endl;
        } else if (field->type() == google::protobuf::FieldDescriptor::TYPE_BOOL) {
            bool value = reflection->GetBool(message, field);
            std::cout << "Value: " << std::boolalpha << value << std::endl;
        }
        // 다른 필드 타입들에 대해서도 비슷한 방식으로 처리할 수 있습니다.
    }
}

int main() {
    MyMessage msg;
    msg.set_id(123);
    msg.set_name("Test");
    msg.set_is_active(true);

    PrintFieldNamesAndValues(msg);
    return 0;
}
