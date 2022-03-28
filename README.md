# Any iterator

Ваша задача написать `any_iterator`, поддерживающий `std::forward_iterator_tag`, `std::bidirectional_iterator_tag`,
`std::random_access_iterator_tag` в качестве параметра.
Это type-erasure обертка над итератором. Интерфейс, который нужно реализовать, находится в [any_iterator.h](./any_iterator.h).
Для примеров использования смотрите тесты в [main.cpp](./main.cpp).
Вам так же необходимо применить small-object оптимизацию при написании данного задания.
 
