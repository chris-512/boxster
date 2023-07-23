

TEST(RayApiTest, CompareWithFuture) {
    // future from a packaged task
    std::packaged_task<int(int)> task(Plus1);
})