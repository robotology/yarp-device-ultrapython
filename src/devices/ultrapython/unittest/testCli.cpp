/*
 * Copyright (C) 2006-2021 Istituto Italiano di Tecnologia (IIT)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include "../../../cli/UltraPythonCli.h"
#include "CliMock.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace testing;

TEST(UltraPyCli, setFeature_ok) {
    IFrameGrabberControlsMock *grabberMock = new IFrameGrabberControlsMock();
    UltraPythonCli mockClient(grabberMock);

    EXPECT_CALL(*grabberMock, setFeature(YARP_FEATURE_BRIGHTNESS_ABSOLUTE, 10))
        .WillOnce(DoAll(Return<bool>(true)));

    bool result = grabberMock->setFeature(YARP_FEATURE_BRIGHTNESS_ABSOLUTE, 10);

    EXPECT_TRUE(result);

    delete grabberMock;
}

TEST(UltraPyCli, getFeature_ok) {
    IFrameGrabberControlsMock *grabberMock = new IFrameGrabberControlsMock();
    UltraPythonCli mockClient(grabberMock);

    double value = 10;

    EXPECT_CALL(*grabberMock, getFeature(YARP_FEATURE_BRIGHTNESS_ABSOLUTE, &value))
        .WillOnce(DoAll(SetArgPointee<1>(value), Return<bool>(true)));

    bool result = grabberMock->getFeature(YARP_FEATURE_BRIGHTNESS_ABSOLUTE, &value);

    EXPECT_EQ(value, 10);
    EXPECT_TRUE(result);
    delete grabberMock;
}

TEST(UltraPyCli, split_string_ok) {
    // use ; as delimiter
    std::string test1 = "a;b";
    std::string test2 = "a;b;c";
    std::string test3 = "a b";
    std::string test4 = "a;b c";
    std::string test5 = "a\nb;c";

    IFrameGrabberControlsMock *grabberMock = new IFrameGrabberControlsMock();
    UltraPythonCli mockClient(grabberMock);

    std::vector<std::string> result1 = mockClient.splitString(test1, ";");
    ASSERT_THAT(result1, ElementsAre("a", "b"));

    std::vector<std::string> result2 = mockClient.splitString(test2, ";");
    ASSERT_THAT(result2, ElementsAre("a", "b", "c"));

    std::vector<std::string> result3 = mockClient.splitString(test3, ";");
    ASSERT_THAT(result3, ElementsAre("a b"));

    std::vector<std::string> result4 = mockClient.splitString(test4, ";");
    ASSERT_THAT(result4, ElementsAre("a", "b c"));

    std::vector<std::string> result5 = mockClient.splitString(test5, ";");
    ASSERT_THAT(result5, ElementsAre("a\nb", "c"));
}
