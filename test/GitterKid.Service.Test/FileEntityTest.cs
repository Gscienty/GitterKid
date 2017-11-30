using System;
using Xunit;
using GitterKid.Service;

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
    }
}
