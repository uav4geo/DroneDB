/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "gtest/gtest.h"
#include "dbops.h"
#include "exceptions.h"
#include "test.h"
#include "testarea.h"

namespace {

using namespace ddb;

TEST(getIndexPathList, includeDirs) {
    auto pathList = ddb::getIndexPathList("data", {(fs::path("data") / "folderA" / "test.txt").string()}, true);
    EXPECT_EQ(pathList.size(), 2);
	EXPECT_TRUE(std::find(pathList.begin(), pathList.end(), fs::path("data") / "folderA" / "test.txt") != pathList.end());
	EXPECT_TRUE(std::find(pathList.begin(), pathList.end(), fs::path("data") / "folderA") != pathList.end());

    pathList = ddb::getIndexPathList(".", {
		(fs::path("data") / "folderA" / "test.txt").string(),
        (fs::path("data") / "folderA" / "folderB" / "test.txt").string()}, true);
    EXPECT_EQ(pathList.size(), 5);
	EXPECT_TRUE(std::find(pathList.begin(), pathList.end(), fs::path("data") / "folderA" / "test.txt") != pathList.end());
	EXPECT_TRUE(std::find(pathList.begin(), pathList.end(), fs::path("data") / "folderA" / "folderB" / "test.txt") != pathList.end());
	EXPECT_TRUE(std::find(pathList.begin(), pathList.end(), fs::path("data") / "folderA") != pathList.end());
	EXPECT_TRUE(std::find(pathList.begin(), pathList.end(), fs::path("data")) != pathList.end());
	EXPECT_TRUE(std::find(pathList.begin(), pathList.end(), fs::path("data") / "folderA" / "folderB") != pathList.end());

    EXPECT_THROW(
    pathList = ddb::getIndexPathList("otherRoot", {
        (fs::path("data") / "folderA" / "test.txt").string(),
    }, true),
    FSException
    );
}

TEST(getIndexPathList, dontIncludeDirs) {
    auto pathList = ddb::getIndexPathList("data", {(fs::path("data") / "folderA" / "test.txt").string()}, false);
    EXPECT_EQ(pathList.size(), 1);
    EXPECT_STREQ(pathList[0].string().c_str(), (fs::path("data") / "folderA" / "test.txt").string().c_str());

}

int countEntries(Database* db, const std::string path)
{
    auto q = db->query("SELECT COUNT(*) FROM entries WHERE Path = ?");
    q->bind(1, path);
    q->fetch();
    const auto cnt = q->getInt(0);
    q->reset();

    return cnt;
}

int countEntries(Database* db)
{
	
    auto q = db->query("SELECT COUNT(*) FROM entries");
    q->fetch();
    const auto cnt = q->getInt(0);
    q->reset();
	
    return cnt;
}

TEST(deleteFromIndex, simplePath) {
    TestArea ta(TEST_NAME);

    const auto sqlite = ta.downloadTestAsset("https://github.com/DroneDB/test_data/raw/master/ddb-remove-test/.ddb/dbase.sqlite", "dbase.sqlite");

    const auto testFolder = ta.getFolder("test");
    create_directory(testFolder / ".ddb");
    fs::copy(sqlite.string(), testFolder / ".ddb", fs::copy_options::overwrite_existing);
    const auto dbPath = testFolder / ".ddb" / "dbase.sqlite";
    EXPECT_TRUE(fs::exists(dbPath));

    Database db;

    db.open(dbPath.string());

    std::vector<std::string> toRemove;
    toRemove.emplace_back((testFolder / "pics.jpg").string());

	removeFromIndex(&db, toRemove);

    EXPECT_EQ(countEntries(&db, "pics.jpg"), 0);
        
    db.close();	
	
}

TEST(deleteFromIndex, folderPath) {
    TestArea ta(TEST_NAME);

    const auto sqlite = ta.downloadTestAsset("https://github.com/DroneDB/test_data/raw/master/ddb-remove-test/.ddb/dbase.sqlite", "dbase.sqlite");

    const auto testFolder = ta.getFolder("test");
    create_directory(testFolder / ".ddb");
    fs::copy(sqlite.string(), testFolder / ".ddb", fs::copy_options::overwrite_existing);
    const auto dbPath = testFolder / ".ddb" / "dbase.sqlite";
    EXPECT_TRUE(fs::exists(dbPath));

    Database db;

    db.open(dbPath.string());

    std::vector<std::string> toRemove;

	// 9
    toRemove.emplace_back((testFolder / "pics").string());

    removeFromIndex(&db, toRemove);
    auto cnt = countEntries(&db);
    EXPECT_EQ(cnt, 15);

    db.close();

}

TEST(deleteFromIndex, subFolderPath) {
    TestArea ta(TEST_NAME);

    const auto sqlite = ta.downloadTestAsset("https://github.com/DroneDB/test_data/raw/master/ddb-remove-test/.ddb/dbase.sqlite", "dbase.sqlite");

    const auto testFolder = ta.getFolder("test");
    create_directory(testFolder / ".ddb");
    fs::copy(sqlite.string(), testFolder / ".ddb", fs::copy_options::overwrite_existing);
    const auto dbPath = testFolder / ".ddb" / "dbase.sqlite";
    EXPECT_TRUE(fs::exists(dbPath));

    Database db;

    db.open(dbPath.string());

    std::vector<std::string> toRemove;
    // 3
    toRemove.emplace_back((testFolder / "pics" / "pics2").string());

    removeFromIndex(&db, toRemove);

    EXPECT_EQ(countEntries(&db), 21);

    db.close();

}

TEST(deleteFromIndex, fileExact) {
    TestArea ta(TEST_NAME);

    const auto sqlite = ta.downloadTestAsset("https://github.com/DroneDB/test_data/raw/master/ddb-remove-test/.ddb/dbase.sqlite", "dbase.sqlite");

    const auto testFolder = ta.getFolder("test");
    create_directory(testFolder / ".ddb");
    fs::copy(sqlite.string(), testFolder / ".ddb", fs::copy_options::overwrite_existing);
    const auto dbPath = testFolder / ".ddb" / "dbase.sqlite";
    EXPECT_TRUE(fs::exists(dbPath));

    Database db;

    db.open(dbPath.string());

    std::vector<std::string> toRemove;
    // 1
    toRemove.emplace_back((testFolder / "1JI_0065.JPG").string());

    removeFromIndex(&db, toRemove);

    EXPECT_EQ(countEntries(&db, "1JI_0065.JPG"), 0);

    db.close();

}


TEST(deleteFromIndex, fileExactInFolder) {
    TestArea ta(TEST_NAME);

    const auto sqlite = ta.downloadTestAsset("https://github.com/DroneDB/test_data/raw/master/ddb-remove-test/.ddb/dbase.sqlite", "dbase.sqlite");

    const auto testFolder = ta.getFolder("test");
    create_directory(testFolder / ".ddb");
    fs::copy(sqlite.string(), testFolder / ".ddb", fs::copy_options::overwrite_existing);
    const auto dbPath = testFolder / ".ddb" / "dbase.sqlite";
    EXPECT_TRUE(fs::exists(dbPath));

    Database db;

    db.open(dbPath.string());

    std::vector<std::string> toRemove;
    // 1
    toRemove.emplace_back((testFolder / "pics" / "IMG_20160826_181309.jpg").string());

    removeFromIndex(&db, toRemove);

    EXPECT_EQ(countEntries(&db, "pics/1JI_0065.JPG"), 0);

    db.close();

}

TEST(deleteFromIndex, fileWildcard) {
    TestArea ta(TEST_NAME);

    const auto sqlite = ta.downloadTestAsset("https://github.com/DroneDB/test_data/raw/master/ddb-remove-test/.ddb/dbase.sqlite", "dbase.sqlite");

    const auto testFolder = ta.getFolder("test");
    create_directory(testFolder / ".ddb");
    fs::copy(sqlite.string(), testFolder / ".ddb", fs::copy_options::overwrite_existing);
    const auto dbPath = testFolder / ".ddb" / "dbase.sqlite";
    EXPECT_TRUE(fs::exists(dbPath));

    Database db;

    db.open(dbPath.string());

    std::vector<std::string> toRemove;
    // 2
    toRemove.emplace_back((testFolder / "1JI*").string());

    removeFromIndex(&db, toRemove);

    EXPECT_EQ(countEntries(&db), 22);

    db.close();

}


TEST(deleteFromIndex, fileInFolderWildcard) {
    TestArea ta(TEST_NAME);

    const auto sqlite = ta.downloadTestAsset("https://github.com/DroneDB/test_data/raw/master/ddb-remove-test/.ddb/dbase.sqlite", "dbase.sqlite");

    const auto testFolder = ta.getFolder("test");
    create_directory(testFolder / ".ddb");
    fs::copy(sqlite.string(), testFolder / ".ddb", fs::copy_options::overwrite_existing);
    const auto dbPath = testFolder / ".ddb" / "dbase.sqlite";
    EXPECT_TRUE(fs::exists(dbPath));

    Database db;

    db.open(dbPath.string());

    std::vector<std::string> toRemove;
    // 5
    toRemove.emplace_back((testFolder / "pics" / "IMG*").string());

    removeFromIndex(&db, toRemove);

    EXPECT_EQ(countEntries(&db), 19);

	EXPECT_EQ(countEntries(&db, "pics/IMG_20160826_181302.jpg"), 0);
    EXPECT_EQ(countEntries(&db, "pics/IMG_20160826_181305.jpg"), 0);
    EXPECT_EQ(countEntries(&db, "pics/IMG_20160826_181309.jpg"), 0);
    EXPECT_EQ(countEntries(&db, "pics/IMG_20160826_181314.jpg"), 0);
    EXPECT_EQ(countEntries(&db, "pics/IMG_20160826_181317.jpg"), 0);

    db.close();

}

TEST(deleteFromIndex, fileExactDirtyDot) {
    TestArea ta(TEST_NAME);

    const auto sqlite = ta.downloadTestAsset("https://github.com/DroneDB/test_data/raw/master/ddb-remove-test/.ddb/dbase.sqlite", "dbase.sqlite");

    const auto testFolder = ta.getFolder("test");
    create_directory(testFolder / ".ddb");
    fs::copy(sqlite.string(), testFolder / ".ddb", fs::copy_options::overwrite_existing);
    const auto dbPath = testFolder / ".ddb" / "dbase.sqlite";
    EXPECT_TRUE(fs::exists(dbPath));

    Database db;

    db.open(dbPath.string());

    std::vector<std::string> toRemove;
    // 1
    toRemove.emplace_back((testFolder / "." / "1JI_0065.JPG").string());

    removeFromIndex(&db, toRemove);

    EXPECT_EQ(countEntries(&db, "1JI_0065.JPG"), 0);

    db.close();

}

TEST(deleteFromIndex, fileExactDirtyDotDot) {
    TestArea ta(TEST_NAME);

    const auto sqlite = ta.downloadTestAsset("https://github.com/DroneDB/test_data/raw/master/ddb-remove-test/.ddb/dbase.sqlite", "dbase.sqlite");

    const auto testFolder = ta.getFolder("test");
    create_directory(testFolder / ".ddb");
    fs::copy(sqlite.string(), testFolder / ".ddb", fs::copy_options::overwrite_existing);
    const auto dbPath = testFolder / ".ddb" / "dbase.sqlite";
    EXPECT_TRUE(fs::exists(dbPath));

    Database db;

    db.open(dbPath.string());

    std::vector<std::string> toRemove;
    // 1
    toRemove.emplace_back((testFolder / "pics" / ".." / "1JI_0065.JPG").string());

    removeFromIndex(&db, toRemove);

    EXPECT_EQ(countEntries(&db, "1JI_0065.JPG"), 0);

    db.close();

}

TEST(listIndex, fileExact) {
    TestArea ta(TEST_NAME);

    const auto sqlite = ta.downloadTestAsset("https://github.com/DroneDB/test_data/raw/master/ddb-remove-test/.ddb/dbase.sqlite", "dbase.sqlite");

    const auto testFolder = ta.getFolder("test");
    create_directory(testFolder / ".ddb");
    fs::copy(sqlite.string(), testFolder / ".ddb", fs::copy_options::overwrite_existing);
    const auto dbPath = testFolder / ".ddb" / "dbase.sqlite";
    EXPECT_TRUE(fs::exists(dbPath));

    Database db;

    db.open(dbPath.string());

    std::vector<std::string> toList;
    
    toList.emplace_back((testFolder / "1JI_0065.JPG").string());

    std::ostringstream out; 

    listIndex(&db, toList, out, "text", false, -1);

    std::cout << out.str() << std::endl;
    EXPECT_EQ(out.str(), "1JI_0065.JPG\n");

    db.close();

}

TEST(listIndex, fileWildcard) {
    TestArea ta(TEST_NAME);

    const auto sqlite = ta.downloadTestAsset("https://github.com/DroneDB/test_data/raw/master/ddb-remove-test/.ddb/dbase.sqlite", "dbase.sqlite");

    const auto testFolder = ta.getFolder("test");
    create_directory(testFolder / ".ddb");
    fs::copy(sqlite.string(), testFolder / ".ddb", fs::copy_options::overwrite_existing);
    const auto dbPath = testFolder / ".ddb" / "dbase.sqlite";
    EXPECT_TRUE(fs::exists(dbPath));

    Database db;

    db.open(dbPath.string());

    std::vector<std::string> toList;
    
    toList.emplace_back((testFolder / "*").string());

    std::ostringstream out; 

    listIndex(&db, toList, out, "text", false, -1);

    std::cout << out.str() << std::endl;
    EXPECT_EQ(out.str(), "1JI_0064.JPG\n1JI_0065.JPG\npics\npics.JPG\npics2\n");

    db.close();

}


TEST(listIndex, emptyPaths) {
    TestArea ta(TEST_NAME);

    const auto sqlite = ta.downloadTestAsset("https://github.com/DroneDB/test_data/raw/master/ddb-remove-test/.ddb/dbase.sqlite", "dbase.sqlite");

    const auto testFolder = ta.getFolder("test");
    create_directory(testFolder / ".ddb");
    fs::copy(sqlite.string(), testFolder / ".ddb", fs::copy_options::overwrite_existing);
    const auto dbPath = testFolder / ".ddb" / "dbase.sqlite";
    EXPECT_TRUE(fs::exists(dbPath));

    Database db;

    db.open(dbPath.string());

    std::vector<std::string> toList;
    
    std::ostringstream out; 

    listIndex(&db, toList, out, "text", false, -1);

    std::cout << out.str() << std::endl;
    EXPECT_EQ(out.str(), "1JI_0064.JPG\n1JI_0065.JPG\npics\npics.JPG\npics2\n");

    db.close();

}


TEST(listIndex, folder) {
    TestArea ta(TEST_NAME);

    const auto sqlite = ta.downloadTestAsset("https://github.com/DroneDB/test_data/raw/master/ddb-remove-test/.ddb/dbase.sqlite", "dbase.sqlite");

    const auto testFolder = ta.getFolder("test");
    create_directory(testFolder / ".ddb");
    fs::copy(sqlite.string(), testFolder / ".ddb", fs::copy_options::overwrite_existing);
    const auto dbPath = testFolder / ".ddb" / "dbase.sqlite";
    EXPECT_TRUE(fs::exists(dbPath));

    Database db;

    db.open(dbPath.string());

    std::vector<std::string> toList;

    toList.emplace_back((testFolder / "pics").string());
    
    std::ostringstream out; 

    listIndex(&db, toList, out, "text", false, -1);

    std::cout << out.str() << std::endl;
    EXPECT_EQ(out.str(), "pics/IMG_20160826_181302.jpg\npics/IMG_20160826_181305.jpg\npics/IMG_20160826_181309.jpg\npics/IMG_20160826_181314.jpg\npics/IMG_20160826_181317.jpg\npics/pics2\n");

    db.close();

}

TEST(listIndex, subFolder) {
    TestArea ta(TEST_NAME);

    const auto sqlite = ta.downloadTestAsset("https://github.com/DroneDB/test_data/raw/master/ddb-remove-test/.ddb/dbase.sqlite", "dbase.sqlite");

    const auto testFolder = ta.getFolder("test");
    create_directory(testFolder / ".ddb");
    fs::copy(sqlite.string(), testFolder / ".ddb", fs::copy_options::overwrite_existing);
    const auto dbPath = testFolder / ".ddb" / "dbase.sqlite";
    EXPECT_TRUE(fs::exists(dbPath));

    Database db;

    db.open(dbPath.string());

    std::vector<std::string> toList;

    toList.emplace_back((testFolder / "pics" / "pics2").string());
    
    std::ostringstream out; 

    listIndex(&db, toList, out, "text", false, -1);

    std::cout << out.str() << std::endl;
    EXPECT_EQ(out.str(), "pics/pics2/IMG_20160826_181305.jpg\npics/pics2/IMG_20160826_181309.jpg\n");

    db.close();

}

TEST(listIndex, fileExactInSubfolder) {
    TestArea ta(TEST_NAME);

    const auto sqlite = ta.downloadTestAsset("https://github.com/DroneDB/test_data/raw/master/ddb-remove-test/.ddb/dbase.sqlite", "dbase.sqlite");

    const auto testFolder = ta.getFolder("test");
    create_directory(testFolder / ".ddb");
    fs::copy(sqlite.string(), testFolder / ".ddb", fs::copy_options::overwrite_existing);
    const auto dbPath = testFolder / ".ddb" / "dbase.sqlite";
    EXPECT_TRUE(fs::exists(dbPath));

    Database db;

    db.open(dbPath.string());

    std::vector<std::string> toList;

    toList.emplace_back((testFolder / "pics" / "IMG_20160826_181314.jpg").string());
    
    std::ostringstream out; 

    listIndex(&db, toList, out, "text", false, -1);

    std::cout << out.str() << std::endl;
    EXPECT_EQ(out.str(), "pics/IMG_20160826_181314.jpg\n");

    db.close();

}

TEST(listIndex, fileExactInSubfolderWithPathToResolve) {
    TestArea ta(TEST_NAME);

    const auto sqlite = ta.downloadTestAsset("https://github.com/DroneDB/test_data/raw/master/ddb-remove-test/.ddb/dbase.sqlite", "dbase.sqlite");

    const auto testFolder = ta.getFolder("test");
    create_directory(testFolder / ".ddb");
    fs::copy(sqlite.string(), testFolder / ".ddb", fs::copy_options::overwrite_existing);
    const auto dbPath = testFolder / ".ddb" / "dbase.sqlite";
    EXPECT_TRUE(fs::exists(dbPath));

    Database db;

    db.open(dbPath.string());

    std::vector<std::string> toList;

    toList.emplace_back((testFolder / "pics2" / ".." / "pics" /"IMG_20160826_181314.jpg").string());
    
    std::ostringstream out; 

    listIndex(&db, toList, out, "text", false, -1);

    std::cout << out.str() << std::endl;
    EXPECT_EQ(out.str(), "pics/IMG_20160826_181314.jpg\n");

    db.close();

}

TEST(listIndex, fileExactInSubfolderWithPathToResolve2) {
    TestArea ta(TEST_NAME);

    const auto sqlite = ta.downloadTestAsset("https://github.com/DroneDB/test_data/raw/master/ddb-remove-test/.ddb/dbase.sqlite", "dbase.sqlite");

    const auto testFolder = ta.getFolder("test");
    create_directory(testFolder / ".ddb");
    fs::copy(sqlite.string(), testFolder / ".ddb", fs::copy_options::overwrite_existing);
    const auto dbPath = testFolder / ".ddb" / "dbase.sqlite";
    EXPECT_TRUE(fs::exists(dbPath));

    Database db;

    db.open(dbPath.string());

    std::vector<std::string> toList;

    toList.emplace_back((testFolder / "pics2" / ".." / "pics" / "." /"IMG_20160826_181314.jpg").string());
    
    std::ostringstream out; 

    listIndex(&db, toList, out, "text", false, -1);

    std::cout << out.str() << std::endl;
    EXPECT_EQ(out.str(), "pics/IMG_20160826_181314.jpg\n");

    db.close();

}

TEST(listIndex, allRecursive) {
    TestArea ta(TEST_NAME);

    const auto sqlite = ta.downloadTestAsset("https://github.com/DroneDB/test_data/raw/master/ddb-remove-test/.ddb/dbase.sqlite", "dbase.sqlite");

    const auto testFolder = ta.getFolder("test");
    create_directory(testFolder / ".ddb");
    fs::copy(sqlite.string(), testFolder / ".ddb", fs::copy_options::overwrite_existing);
    const auto dbPath = testFolder / ".ddb" / "dbase.sqlite";
    EXPECT_TRUE(fs::exists(dbPath));

    Database db;

    db.open(dbPath.string());

    std::vector<std::string> toList;

    toList.emplace_back((testFolder).string());
    
    std::ostringstream out; 

    listIndex(&db, toList, out, "text", true, -1);

    std::cout << out.str() << std::endl;
    EXPECT_EQ(out.str(), "1JI_0064.JPG\n1JI_0065.JPG\npics\npics.JPG\npics/IMG_20160826_181302.jpg\npics/IMG_20160826_181305.jpg\npics/IMG_20160826_181309.jpg\npics/IMG_20160826_181314.jpg\npics/IMG_20160826_181317.jpg\npics/pics2\npics/pics2/IMG_20160826_181305.jpg\npics/pics2/IMG_20160826_181309.jpg\npics2\npics2/IMG_20160826_181305.jpg\npics2/IMG_20160826_181309.jpg\npics2/pics\npics2/pics/IMG_20160826_181302.jpg\npics2/pics/IMG_20160826_181305.jpg\npics2/pics/IMG_20160826_181309.jpg\npics2/pics/IMG_20160826_181314.jpg\npics2/pics/IMG_20160826_181317.jpg\npics2/pics/pics2\npics2/pics/pics2/IMG_20160826_181305.jpg\npics2/pics/pics2/IMG_20160826_181309.jpg\n");

    db.close();

}

TEST(listIndex, folderRecursive) {
    TestArea ta(TEST_NAME);

    const auto sqlite = ta.downloadTestAsset("https://github.com/DroneDB/test_data/raw/master/ddb-remove-test/.ddb/dbase.sqlite", "dbase.sqlite");

    const auto testFolder = ta.getFolder("test");
    create_directory(testFolder / ".ddb");
    fs::copy(sqlite.string(), testFolder / ".ddb", fs::copy_options::overwrite_existing);
    const auto dbPath = testFolder / ".ddb" / "dbase.sqlite";
    EXPECT_TRUE(fs::exists(dbPath));

    Database db;

    db.open(dbPath.string());

    std::vector<std::string> toList;

    toList.emplace_back((testFolder / "pics").string());
    
    std::ostringstream out; 

    listIndex(&db, toList, out, "text", true, -1);

    std::cout << out.str() << std::endl;
    EXPECT_EQ(out.str(), "pics/IMG_20160826_181302.jpg\npics/IMG_20160826_181305.jpg\npics/IMG_20160826_181309.jpg\npics/IMG_20160826_181314.jpg\npics/IMG_20160826_181317.jpg\npics/pics2\npics/pics2/IMG_20160826_181305.jpg\npics/pics2/IMG_20160826_181309.jpg\n");

    db.close();

}

TEST(listIndex, folderRecursiveWithLimit) {
    TestArea ta(TEST_NAME);

    const auto sqlite = ta.downloadTestAsset("https://github.com/DroneDB/test_data/raw/master/ddb-remove-test/.ddb/dbase.sqlite", "dbase.sqlite");

    const auto testFolder = ta.getFolder("test");
    create_directory(testFolder / ".ddb");
    fs::copy(sqlite.string(), testFolder / ".ddb", fs::copy_options::overwrite_existing);
    const auto dbPath = testFolder / ".ddb" / "dbase.sqlite";
    EXPECT_TRUE(fs::exists(dbPath));

    Database db;

    db.open(dbPath.string());

    std::vector<std::string> toList;

    toList.emplace_back((testFolder / "pics").string());
    
    std::ostringstream out; 

    listIndex(&db, toList, out, "text", true, 1);

    std::cout << out.str() << std::endl;
    EXPECT_EQ(out.str(), "pics/IMG_20160826_181302.jpg\npics/IMG_20160826_181305.jpg\npics/IMG_20160826_181309.jpg\npics/IMG_20160826_181314.jpg\npics/IMG_20160826_181317.jpg\npics/pics2\n");

    db.close();

}

TEST(listIndex, wildcardRecursive) {
    TestArea ta(TEST_NAME);

    const auto sqlite = ta.downloadTestAsset("https://github.com/DroneDB/test_data/raw/master/ddb-remove-test/.ddb/dbase.sqlite", "dbase.sqlite");

    const auto testFolder = ta.getFolder("test");
    create_directory(testFolder / ".ddb");
    fs::copy(sqlite.string(), testFolder / ".ddb", fs::copy_options::overwrite_existing);
    const auto dbPath = testFolder / ".ddb" / "dbase.sqlite";
    EXPECT_TRUE(fs::exists(dbPath));

    Database db;

    db.open(dbPath.string());

    std::vector<std::string> toList;

    toList.emplace_back((testFolder / "pics*").string());
    
    std::ostringstream out; 

    listIndex(&db, toList, out, "text", true, -1);

    std::cout << out.str() << std::endl;
    EXPECT_EQ(out.str(), "pics\npics.JPG\npics/IMG_20160826_181302.jpg\npics/IMG_20160826_181305.jpg\npics/IMG_20160826_181309.jpg\npics/IMG_20160826_181314.jpg\npics/IMG_20160826_181317.jpg\npics/pics2\npics/pics2/IMG_20160826_181305.jpg\npics/pics2/IMG_20160826_181309.jpg\npics2\npics2/IMG_20160826_181305.jpg\npics2/IMG_20160826_181309.jpg\npics2/pics\npics2/pics/IMG_20160826_181302.jpg\npics2/pics/IMG_20160826_181305.jpg\npics2/pics/IMG_20160826_181309.jpg\npics2/pics/IMG_20160826_181314.jpg\npics2/pics/IMG_20160826_181317.jpg\npics2/pics/pics2\npics2/pics/pics2/IMG_20160826_181305.jpg\npics2/pics/pics2/IMG_20160826_181309.jpg\n");

    db.close();

}

TEST(listIndex, wildcardRecursiveWithLimit) {
    TestArea ta(TEST_NAME);

    const auto sqlite = ta.downloadTestAsset("https://github.com/DroneDB/test_data/raw/master/ddb-remove-test/.ddb/dbase.sqlite", "dbase.sqlite");

    const auto testFolder = ta.getFolder("test");
    create_directory(testFolder / ".ddb");
    fs::copy(sqlite.string(), testFolder / ".ddb", fs::copy_options::overwrite_existing);
    const auto dbPath = testFolder / ".ddb" / "dbase.sqlite";
    EXPECT_TRUE(fs::exists(dbPath));

    Database db;

    db.open(dbPath.string());

    std::vector<std::string> toList;

    toList.emplace_back((testFolder / "pics*").string());
    
    std::ostringstream out; 

    listIndex(&db, toList, out, "text", true, 1);

    std::cout << out.str() << std::endl;
    EXPECT_EQ(out.str(), "pics\npics.JPG\npics/IMG_20160826_181302.jpg\npics/IMG_20160826_181305.jpg\npics/IMG_20160826_181309.jpg\npics/IMG_20160826_181314.jpg\npics/IMG_20160826_181317.jpg\npics/pics2\npics2\npics2/IMG_20160826_181305.jpg\npics2/IMG_20160826_181309.jpg\npics2/pics\n");

    db.close();

}

}
