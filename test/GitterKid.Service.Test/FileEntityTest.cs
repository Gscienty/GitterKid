using System;
using Xunit;
using GitterKid.Service;
using GitterKid.Service.Extension;

namespace GitterKid.Service.Test
{
    public class FileEntityTest
    {
        [Theory]
        [InlineData("bf3f8339277ac8e1996c14c89012afae40bfa9b9")]
        public void TreeEntityTest(string signture)
        {
            Repository repository = new Repository(@"../../../../../.git/");
            Assert.True(repository.Entity<GitTree>(signture).Exist(".gitignore"));
            Assert.True(repository.Entity<GitTree>(signture).Exist("src"));
            Assert.True(repository.Entity<GitTree>(signture).Exist("test"));

            Assert.False(repository.Entity<GitTree>(signture).Exist("is not exist"));

            Assert.True(repository.Entity<GitTree>(signture).Open<GitTree>("test").Exist("GitterKid.Service.Test"));
        }

        [Theory]
        [InlineData("1bf4df06e529c3174f45f8a405d1b4aaeb5f1e14")]
        public void CommitEntityTest(string signture)
        {
            Repository repository = new Repository(@"../../../../../.git/");
            Assert.Equal(repository.Entity<GitCommit>(signture).TreeSignture, "b3b66397ffe49feca3920a8918f99849741d415d");
            Assert.Equal(repository.Entity<GitCommit>(signture).Author.Mail, "gaoxiaochuan@hotmail.com");
        }

        [Fact]
        public void BranchTest()
        {
            Repository repository = new Repository(@"../../../../../.git/");
            Assert.True(repository.ExistBranch("master"));
            Assert.Equal(repository.Branches["master"].BranchLogs[0].Signture, "38305922037678e0b79ff4dd4ff81b09a408065a");
        }

        [Fact]
        public void CompareBlob()
        {
            Repository repository = new Repository(@"../../../../../.git/");

            GitBlob currentBlob = repository.Entity<GitBlob>("f21cf96546031d9cacbce76bffef705d5d6928b3");
            GitBlob originBlob = repository.Entity<GitBlob>("411421fe177c328b971a27cf25a377e12c90f10e");

            (int AddedCount, int DeletedCount) compareResult = currentBlob.Compare(originBlob);

            Assert.Equal(compareResult.AddedCount, 18);
            Assert.Equal(compareResult.DeletedCount, 1);
        }
    }
}
